#include "audiomanager.h"

#include <QAudioOutput>
#include <QDebug>
#include <QDir>
#include <QEasingCurve>
#include <QFile>
#include <QFileInfo>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QMediaPlayer>
#include <QPropertyAnimation>
#include <QTimer>
#include <QUrl>
#include <QVariantAnimation>

namespace {

AudioCategory parseCategory(const QString &value, bool *ok)
{
    const QString normalized = value.trimmed().toLower();
    if (normalized == QStringLiteral("bgm")) {
        *ok = true;
        return AudioCategory::Bgm;
    }
    if (normalized == QStringLiteral("amb")) {
        *ok = true;
        return AudioCategory::Amb;
    }
    if (normalized == QStringLiteral("sfx")) {
        *ok = true;
        return AudioCategory::Sfx;
    }
    if (normalized == QStringLiteral("ui")) {
        *ok = true;
        return AudioCategory::Ui;
    }
    if (normalized == QStringLiteral("emo")) {
        *ok = true;
        return AudioCategory::Emo;
    }

    *ok = false;
    return AudioCategory::Sfx;
}

QString resolvedAudioPath(const QString &manifestPath, const QString &relativePath)
{
    return QDir(QFileInfo(manifestPath).absolutePath()).filePath(relativePath);
}

} // namespace

AudioManager::AudioManager(QObject *parent)
    : QObject(parent)
{
    m_bgmChannel.player = new QMediaPlayer(this);
    m_bgmChannel.output = new QAudioOutput(this);
    m_bgmChannel.fadeAnimation = new QPropertyAnimation(m_bgmChannel.output, "volume", this);
    m_bgmChannel.fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_bgmChannel.output->setVolume(1.0);
    m_bgmChannel.player->setAudioOutput(m_bgmChannel.output);

    m_ambChannel.player = new QMediaPlayer(this);
    m_ambChannel.output = new QAudioOutput(this);
    m_ambChannel.fadeAnimation = new QPropertyAnimation(m_ambChannel.output, "volume", this);
    m_ambChannel.fadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    m_ambChannel.output->setVolume(1.0);
    m_ambChannel.player->setAudioOutput(m_ambChannel.output);
}

AudioManager::~AudioManager() = default;

bool AudioManager::loadManifest(const QString &manifestPath)
{
    QFile file(manifestPath);
    if (!file.open(QIODevice::ReadOnly)) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Failed to open manifest: %1").arg(manifestPath);
        return false;
    }

    QJsonParseError error;
    const QJsonDocument document = QJsonDocument::fromJson(file.readAll(), &error);
    if (error.error != QJsonParseError::NoError || !document.isObject()) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Invalid manifest JSON: %1 (%2)")
                                .arg(manifestPath, error.errorString());
        return false;
    }

    const QJsonArray entries = document.object().value(QStringLiteral("entries")).toArray();
    QHash<QString, AudioManifestEntry> loadedEntries;

    for (const QJsonValue &value : entries) {
        if (!value.isObject()) {
            continue;
        }

        const QJsonObject object = value.toObject();
        const QString id = object.value(QStringLiteral("id")).toString().trimmed();
        const QString path = object.value(QStringLiteral("path")).toString().trimmed();
        const QString categoryText = object.value(QStringLiteral("category")).toString();
        bool categoryOk = false;
        const AudioCategory category = parseCategory(categoryText, &categoryOk);

        if (id.isEmpty() || path.isEmpty() || !categoryOk) {
            qWarning().noquote() << QStringLiteral("[AudioManager] Skipped invalid manifest entry id=%1 category=%2 path=%3")
                                    .arg(id, categoryText, path);
            continue;
        }

        AudioManifestEntry entry;
        entry.id = id;
        entry.category = category;
        entry.relativePath = path;
        entry.resolvedPath = resolvedAudioPath(manifestPath, path);
        entry.loop = object.value(QStringLiteral("loop")).toBool(category == AudioCategory::Bgm
                                                                 || category == AudioCategory::Amb);
        entry.volume = qBound<qreal>(0.0, object.value(QStringLiteral("volume")).toDouble(1.0), 1.0);

        loadedEntries.insert(id, entry);
    }

    m_entriesById = loadedEntries;
    m_loadedManifestPath = manifestPath;
    qInfo().noquote() << QStringLiteral("[AudioManager] Loaded manifest %1 (%2 entries)")
                          .arg(manifestPath)
                          .arg(m_entriesById.size());
    return !m_entriesById.isEmpty();
}

QString AudioManager::loadedManifestPath() const
{
    return m_loadedManifestPath;
}

QStringList AudioManager::availableIds(AudioCategory category) const
{
    QStringList ids;
    for (auto it = m_entriesById.cbegin(); it != m_entriesById.cend(); ++it) {
        if (it.value().category == category) {
            ids.append(it.key());
        }
    }

    ids.sort();
    return ids;
}

bool AudioManager::hasAudio(const QString &audioId) const
{
    return m_entriesById.contains(audioId);
}

QString AudioManager::currentBgmId() const
{
    return m_bgmChannel.currentAudioId;
}

QString AudioManager::currentAmbId() const
{
    return m_ambChannel.currentAudioId;
}

void AudioManager::playBgm(const QString &audioId, int fadeOutMs, int fadeInMs)
{
    transitionLoopingChannel(&m_bgmChannel, audioId, AudioCategory::Bgm, fadeOutMs, fadeInMs);
}

void AudioManager::stopBgm(int fadeOutMs)
{
    fadeOutAndStop(&m_bgmChannel, fadeOutMs);
}

void AudioManager::duckBgm(qreal volumeScale, int fadeMs)
{
    if (m_bgmChannel.currentAudioId.isEmpty()) {
        return;
    }

    const AudioManifestEntry entry = entryForId(m_bgmChannel.currentAudioId);
    if (entry.id.isEmpty()) {
        return;
    }

    m_bgmChannel.volumeScale = qBound<qreal>(0.0, volumeScale, 1.0);
    animateChannelVolume(&m_bgmChannel, entry.volume * m_bgmChannel.volumeScale, fadeMs);
}

void AudioManager::restoreBgm(int fadeMs)
{
    if (m_bgmChannel.currentAudioId.isEmpty()) {
        return;
    }

    const AudioManifestEntry entry = entryForId(m_bgmChannel.currentAudioId);
    if (entry.id.isEmpty()) {
        return;
    }

    m_bgmChannel.volumeScale = 1.0;
    animateChannelVolume(&m_bgmChannel, entry.volume, fadeMs);
}

void AudioManager::playAmb(const QString &audioId, int fadeOutMs, int fadeInMs)
{
    transitionLoopingChannel(&m_ambChannel, audioId, AudioCategory::Amb, fadeOutMs, fadeInMs);
}

void AudioManager::stopAmb(int fadeOutMs)
{
    fadeOutAndStop(&m_ambChannel, fadeOutMs);
}

void AudioManager::setAmbVolumeScale(qreal volumeScale, int fadeMs)
{
    if (m_ambChannel.currentAudioId.isEmpty()) {
        return;
    }

    const AudioManifestEntry entry = entryForId(m_ambChannel.currentAudioId);
    if (entry.id.isEmpty()) {
        return;
    }

    m_ambChannel.volumeScale = qBound<qreal>(0.0, volumeScale, 2.0);
    animateChannelVolume(&m_ambChannel,
                         qBound<qreal>(0.0, entry.volume * m_ambChannel.volumeScale, 1.0),
                         fadeMs);
}

void AudioManager::restoreAmb(int fadeMs)
{
    if (m_ambChannel.currentAudioId.isEmpty()) {
        return;
    }

    const AudioManifestEntry entry = entryForId(m_ambChannel.currentAudioId);
    if (entry.id.isEmpty()) {
        return;
    }

    m_ambChannel.volumeScale = 1.0;
    animateChannelVolume(&m_ambChannel, entry.volume, fadeMs);
}

void AudioManager::playSfx(const QString &audioId)
{
    playOneShot(audioId, AudioCategory::Sfx);
}

void AudioManager::stopSfx(const QString &audioId)
{
    for (int index = m_oneShotChannels.size() - 1; index >= 0; --index) {
        const OneShotChannel &channel = m_oneShotChannels.at(index);
        if (channel.category != AudioCategory::Sfx) {
            continue;
        }
        if (!audioId.isEmpty() && channel.audioId != audioId) {
            continue;
        }
        if (channel.player) {
            channel.player->stop();
        }
    }
}

void AudioManager::playUi(const QString &audioId)
{
    playOneShot(audioId, AudioCategory::Ui);
}

void AudioManager::playEmo(const QString &audioId)
{
    playOneShot(audioId, AudioCategory::Emo);
}

void AudioManager::stopEmo(const QString &audioId)
{
    for (int index = m_oneShotChannels.size() - 1; index >= 0; --index) {
        const OneShotChannel &channel = m_oneShotChannels.at(index);
        if (channel.category != AudioCategory::Emo) {
            continue;
        }
        if (!audioId.isEmpty() && channel.audioId != audioId) {
            continue;
        }
        if (channel.player) {
            channel.player->stop();
        }
    }
}

void AudioManager::stopAll(int fadeOutMs)
{
    stopBgm(fadeOutMs);
    stopAmb(fadeOutMs);
    stopSfx();
}

AudioManifestEntry AudioManager::entryForId(const QString &audioId) const
{
    return m_entriesById.value(audioId);
}

void AudioManager::playOneShot(const QString &audioId, AudioCategory expectedCategory)
{
    const AudioManifestEntry entry = entryForId(audioId);
    if (entry.id.isEmpty()) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Unknown audio id: %1").arg(audioId);
        return;
    }

    if (entry.category != expectedCategory) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Audio id %1 is category %2, not %3")
                                .arg(audioId, categoryName(entry.category), categoryName(expectedCategory));
        return;
    }

    if (!QFileInfo::exists(entry.resolvedPath)) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Missing audio file for %1: %2")
                                .arg(audioId, entry.resolvedPath);
        return;
    }

    auto *player = new QMediaPlayer(this);
    auto *output = new QAudioOutput(this);
    output->setVolume(entry.volume);
    player->setAudioOutput(output);
    player->setLoops(1);
    player->setSource(QUrl::fromLocalFile(entry.resolvedPath));
    m_oneShotChannels.append({player, output, expectedCategory, audioId});

    connect(player, &QMediaPlayer::errorChanged, this, [audioId, player]() {
        if (player->error() != QMediaPlayer::NoError) {
            qWarning().noquote() << QStringLiteral("[AudioManager] Failed to play %1: %2")
                                    .arg(audioId, player->errorString());
        }
    });

    connect(player, &QMediaPlayer::playbackStateChanged, this, [this, player, output](QMediaPlayer::PlaybackState state) {
        if (state == QMediaPlayer::StoppedState) {
            for (int index = m_oneShotChannels.size() - 1; index >= 0; --index) {
                if (m_oneShotChannels.at(index).player == player) {
                    m_oneShotChannels.removeAt(index);
                    break;
                }
            }
            player->deleteLater();
            output->deleteLater();
        }
    });

    player->play();
}

void AudioManager::transitionLoopingChannel(LoopingChannel *channel,
                                            const QString &audioId,
                                            AudioCategory expectedCategory,
                                            int fadeOutMs,
                                            int fadeInMs)
{
    if (!channel) {
        return;
    }

    if (audioId.trimmed().isEmpty()) {
        fadeOutAndStop(channel, fadeOutMs);
        return;
    }

    const AudioManifestEntry entry = entryForId(audioId);
    if (entry.id.isEmpty()) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Unknown audio id: %1").arg(audioId);
        return;
    }

    if (entry.category != expectedCategory) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Audio id %1 is category %2, not %3")
                                .arg(audioId, categoryName(entry.category), categoryName(expectedCategory));
        return;
    }

    if (!QFileInfo::exists(entry.resolvedPath)) {
        qWarning().noquote() << QStringLiteral("[AudioManager] Missing audio file for %1: %2")
                                .arg(audioId, entry.resolvedPath);
        return;
    }

    if (channel->currentAudioId == audioId
        && channel->player->source() == QUrl::fromLocalFile(entry.resolvedPath)) {
        cancelFade(channel);
        channel->output->setVolume(entry.volume * channel->volumeScale);
        if (channel->player->playbackState() != QMediaPlayer::PlayingState) {
            channel->player->play();
        }
        return;
    }

    const int transitionToken = ++channel->transitionToken;
    if (channel->player->playbackState() == QMediaPlayer::StoppedState || channel->currentAudioId.isEmpty()) {
        startLoopingEntry(channel, entry, fadeInMs);
        return;
    }

    cancelFade(channel);
    channel->fadeAnimation->disconnect();
    channel->fadeAnimation->setStartValue(channel->output->volume());
    channel->fadeAnimation->setEndValue(0.0);
    channel->fadeAnimation->setDuration(qMax(0, fadeOutMs));
    connect(channel->fadeAnimation, &QPropertyAnimation::finished, this, [this, channel, entry, fadeInMs, transitionToken]() {
        if (channel->transitionToken != transitionToken) {
            return;
        }
        startLoopingEntry(channel, entry, fadeInMs);
    });
    channel->fadeAnimation->start();
}

void AudioManager::startLoopingEntry(LoopingChannel *channel, const AudioManifestEntry &entry, int fadeInMs)
{
    if (!channel) {
        return;
    }

    cancelFade(channel);
    channel->player->stop();
    channel->currentAudioId = entry.id;
    channel->volumeScale = 1.0;
    channel->player->setSource(QUrl::fromLocalFile(entry.resolvedPath));
    channel->player->setLoops(entry.loop ? QMediaPlayer::Infinite : 1);
    channel->output->setVolume(fadeInMs > 0 ? 0.0 : entry.volume * channel->volumeScale);
    channel->player->play();

    if (fadeInMs <= 0) {
        channel->output->setVolume(entry.volume * channel->volumeScale);
        return;
    }

    channel->fadeAnimation->disconnect();
    channel->fadeAnimation->setStartValue(0.0);
    channel->fadeAnimation->setEndValue(entry.volume * channel->volumeScale);
    channel->fadeAnimation->setDuration(fadeInMs);
    channel->fadeAnimation->start();
}

void AudioManager::fadeOutAndStop(LoopingChannel *channel, int fadeOutMs)
{
    if (!channel) {
        return;
    }

    ++channel->transitionToken;

    if (channel->player->playbackState() == QMediaPlayer::StoppedState) {
        channel->currentAudioId.clear();
        channel->volumeScale = 1.0;
        cancelFade(channel);
        return;
    }

    cancelFade(channel);
    if (fadeOutMs <= 0) {
        channel->player->stop();
        channel->currentAudioId.clear();
        channel->volumeScale = 1.0;
        channel->output->setVolume(1.0);
        return;
    }

    channel->fadeAnimation->disconnect();
    channel->fadeAnimation->setStartValue(channel->output->volume());
    channel->fadeAnimation->setEndValue(0.0);
    channel->fadeAnimation->setDuration(fadeOutMs);
    connect(channel->fadeAnimation, &QPropertyAnimation::finished, this, [channel]() {
        channel->player->stop();
        channel->currentAudioId.clear();
        channel->volumeScale = 1.0;
        channel->output->setVolume(1.0);
    });
    channel->fadeAnimation->start();
}

void AudioManager::animateChannelVolume(LoopingChannel *channel, qreal targetVolume, int fadeMs)
{
    if (!channel || !channel->output) {
        return;
    }

    cancelFade(channel);
    if (fadeMs <= 0) {
        channel->output->setVolume(targetVolume);
        return;
    }

    channel->fadeAnimation->disconnect();
    channel->fadeAnimation->setStartValue(channel->output->volume());
    channel->fadeAnimation->setEndValue(targetVolume);
    channel->fadeAnimation->setDuration(fadeMs);
    channel->fadeAnimation->start();
}

void AudioManager::cancelFade(LoopingChannel *channel)
{
    if (!channel || !channel->fadeAnimation) {
        return;
    }

    channel->fadeAnimation->stop();
}

QString AudioManager::categoryName(AudioCategory category)
{
    switch (category) {
    case AudioCategory::Bgm:
        return QStringLiteral("BGM");
    case AudioCategory::Amb:
        return QStringLiteral("AMB");
    case AudioCategory::Sfx:
        return QStringLiteral("SFX");
    case AudioCategory::Ui:
        return QStringLiteral("UI");
    case AudioCategory::Emo:
        return QStringLiteral("EMO");
    }

    return QStringLiteral("Unknown");
}

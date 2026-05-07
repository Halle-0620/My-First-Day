#ifndef AUDIOMANAGER_H
#define AUDIOMANAGER_H

#include <QHash>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>

class QAudioOutput;
class QMediaPlayer;
class QPropertyAnimation;

enum class AudioCategory {
    Bgm,
    Amb,
    Sfx,
    Ui,
    Emo
};

struct AudioManifestEntry
{
    QString id;
    AudioCategory category = AudioCategory::Sfx;
    QString relativePath;
    QString resolvedPath;
    bool loop = false;
    qreal volume = 1.0;
};

class AudioManager : public QObject
{
    Q_OBJECT

public:
    explicit AudioManager(QObject *parent = nullptr);
    ~AudioManager() override;

    bool loadManifest(const QString &manifestPath);
    QString loadedManifestPath() const;
    QStringList availableIds(AudioCategory category) const;
    bool hasAudio(const QString &audioId) const;
    QString currentBgmId() const;
    QString currentAmbId() const;

    void playBgm(const QString &audioId, int fadeOutMs = 500, int fadeInMs = 700);
    void stopBgm(int fadeOutMs = 400);
    void duckBgm(qreal volumeScale = 0.18, int fadeMs = 400);
    void restoreBgm(int fadeMs = 400);
    void playAmb(const QString &audioId, int fadeOutMs = 500, int fadeInMs = 700);
    void stopAmb(int fadeOutMs = 400);
    void setAmbVolumeScale(qreal volumeScale, int fadeMs = 400);
    void restoreAmb(int fadeMs = 400);
    void playSfx(const QString &audioId);
    void stopSfx(const QString &audioId = QString());
    void playUi(const QString &audioId);
    void playEmo(const QString &audioId);
    void stopAll(int fadeOutMs = 250);

private:
    struct LoopingChannel {
        QMediaPlayer *player = nullptr;
        QAudioOutput *output = nullptr;
        QPropertyAnimation *fadeAnimation = nullptr;
        QString currentAudioId;
        int transitionToken = 0;
        qreal volumeScale = 1.0;
    };

    struct OneShotChannel {
        QMediaPlayer *player = nullptr;
        QAudioOutput *output = nullptr;
        AudioCategory category = AudioCategory::Sfx;
        QString audioId;
    };

    AudioManifestEntry entryForId(const QString &audioId) const;
    void playOneShot(const QString &audioId, AudioCategory expectedCategory);
    void transitionLoopingChannel(LoopingChannel *channel,
                                  const QString &audioId,
                                  AudioCategory expectedCategory,
                                  int fadeOutMs,
                                  int fadeInMs);
    void startLoopingEntry(LoopingChannel *channel, const AudioManifestEntry &entry, int fadeInMs);
    void fadeOutAndStop(LoopingChannel *channel, int fadeOutMs);
    void animateChannelVolume(LoopingChannel *channel, qreal targetVolume, int fadeMs);
    void cancelFade(LoopingChannel *channel);
    static QString categoryName(AudioCategory category);

    QHash<QString, AudioManifestEntry> m_entriesById;
    QString m_loadedManifestPath;
    LoopingChannel m_bgmChannel;
    LoopingChannel m_ambChannel;
    QList<OneShotChannel> m_oneShotChannels;
};

#endif // AUDIOMANAGER_H

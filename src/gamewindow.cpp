#include "gamewindow.h"

#include "audio/audiomanager.h"
#include "audio/audiotestdialog.h"
#include "narrative/samplestory.h"
#include "narrative/storyloader.h"
#include "ui/dialoguepanel.h"
#include "ui/shadertoywidget.h"

#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QMenuBar>
#include <QPixmap>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace {

QPixmap trimTransparentMargins(const QPixmap &pixmap)
{
    if (pixmap.isNull()) {
        return pixmap;
    }

    const QImage image = pixmap.toImage().convertToFormat(QImage::Format_ARGB32);
    int left = image.width();
    int top = image.height();
    int right = -1;
    int bottom = -1;

    for (int y = 0; y < image.height(); ++y) {
        const QRgb *line = reinterpret_cast<const QRgb *>(image.constScanLine(y));
        for (int x = 0; x < image.width(); ++x) {
            if (qAlpha(line[x]) == 0) {
                continue;
            }

            left = qMin(left, x);
            top = qMin(top, y);
            right = qMax(right, x);
            bottom = qMax(bottom, y);
        }
    }

    if (right < left || bottom < top) {
        return pixmap;
    }

    return pixmap.copy(left, top, right - left + 1, bottom - top + 1);
}

QStringList imageNamesForBackground(BackgroundStyle style)
{
    switch (style) {
    case BackgroundStyle::DeskDusk:
        return {QString::fromUtf8(u8"课桌图(傍晚）.png")};
    case BackgroundStyle::DeskNight:
        return {QString::fromUtf8(u8"课桌图（晚上）.png")};
    case BackgroundStyle::Hallway:
        return {QString::fromUtf8(u8"楼道图.png")};
    case BackgroundStyle::TreeUnder:
        return {QString::fromUtf8(u8"树下.png")};
    case BackgroundStyle::GymBack:
        return {QString::fromUtf8(u8"体育馆.png")};
    case BackgroundStyle::Office:
        return {QString::fromUtf8(u8"办公室.png")};
    case BackgroundStyle::Toilet:
        return {QString::fromUtf8(u8"厕所.png")};
    case BackgroundStyle::Dismissal:
        return {QString::fromUtf8(u8"放学.png")};
    case BackgroundStyle::SchoolGate:
        return {QString::fromUtf8(u8"校门口.png")};
    case BackgroundStyle::Ebike:
        return {QString::fromUtf8(u8"电瓶车上.png")};
    case BackgroundStyle::Home:
        return {QString::fromUtf8(u8"家里.png")};
    case BackgroundStyle::ClassroomDusk:
        return {QString::fromUtf8(u8"课桌图(傍晚）.png")};
    case BackgroundStyle::QuietHallway:
        return {QString::fromUtf8(u8"楼道图.png")};
    case BackgroundStyle::ChoiceFocus:
        return {QString::fromUtf8(u8"课桌图（晚上）.png")};
    case BackgroundStyle::SoftNarration:
        return {QString::fromUtf8(u8"课桌图（晚上）.png")};
    case BackgroundStyle::NightRain:
        return {QString::fromUtf8(u8"电瓶车上.png")};
    case BackgroundStyle::EndingGlow:
        return {QString::fromUtf8(u8"树下.png"), QString::fromUtf8(u8"体育馆.png")};
    case BackgroundStyle::EndingBlack:
    case BackgroundStyle::DreamDrift:
        return {};
    case BackgroundStyle::Message1:
        return {QString::fromUtf8(u8"消息1.png")};
    case BackgroundStyle::Message2:
        return {QString::fromUtf8(u8"消息2.png")};
    case BackgroundStyle::Message3:
        return {QString::fromUtf8(u8"消息3.png")};
    }

    return {};
}

QStringList backgroundSearchRoots()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    return {
        QDir(appDir).filePath(QString::fromUtf8(u8"../图")),
        QDir(appDir).filePath(QString::fromUtf8(u8"图")),
        QDir(appDir).filePath(QString::fromUtf8(u8"../../图"))
    };
}

QString ambientAudioIdForScene(const QString &sceneId)
{
    if (sceneId == QStringLiteral("scene9_bell")) {
        return {};
    }

    if (sceneId == QStringLiteral("scene2_report_stuck")) {
        return QStringLiteral("amb_classroom_night_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene1_"))
        || sceneId.startsWith(QStringLiteral("scene2_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene3_"))) {
        return QStringLiteral("amb_classroom_night_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene4_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene5_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("scene6_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId == QStringLiteral("scene7a_opening")
        || sceneId == QStringLiteral("scene7a_hide_place")) {
        return QStringLiteral("amb_playground_night");
    }

    if (sceneId == QStringLiteral("scene7a_topics")
        || sceneId == QStringLiteral("scene7a_hug")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3")) {
        return QStringLiteral("amb_hidden_place_night");
    }

    if (sceneId.startsWith(QStringLiteral("scene7b_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene8_"))) {
        return QStringLiteral("amb_office");
    }

    if (sceneId.startsWith(QStringLiteral("scene9_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene10_"))) {
        return QStringLiteral("amb_rain_road");
    }

    if (sceneId.startsWith(QStringLiteral("scene11_"))
        || sceneId.startsWith(QStringLiteral("scene12_"))) {
        return QStringLiteral("amb_home_night");
    }

    return {};
}

QString bgmAudioIdForScene(const NarrativeViewState &state, const GameState &gameState)
{
    const QString &sceneId = state.sceneId;
    const bool stayedInClassroomRoute = !gameState.boolValue(QStringLiteral("went_downstairs"));

    if (sceneId.startsWith(QStringLiteral("scene1_"))
        || state.sceneId.startsWith(QStringLiteral("scene2_"))
        || state.sceneId.startsWith(QStringLiteral("scene3_"))
        || state.sceneId.startsWith(QStringLiteral("scene4_"))
        || state.sceneId.startsWith(QStringLiteral("scene5_"))) {
        return QStringLiteral("bgm_chunfeng_chenzui");
    }

    if (sceneId.startsWith(QStringLiteral("scene6_"))) {
        return {};
    }

    if (sceneId == QStringLiteral("scene7a_hug")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3")) {
        return QStringLiteral("bgm_jiaomiansi");
    }

    if (sceneId == QStringLiteral("scene9_mainline_end")) {
        return QStringLiteral("bgm_pingfan_zhilu");
    }

    if (sceneId.startsWith(QStringLiteral("scene10_"))) {
        if (sceneId == QStringLiteral("scene10_song_performance")) {
            if (!state.text.contains(QString::fromUtf8(u8"然后你开始唱"))) {
                return {};
            }
            if (gameState.stringValue(QStringLiteral("rain_song_emotion")) == QString::fromUtf8(u8"家庭")) {
                return QStringLiteral("bgm_dont_cry");
            }
            return QStringLiteral("bgm_meiyoulixiang");
        }
        if (sceneId == QStringLiteral("scene10_song_release")
            || sceneId == QStringLiteral("scene10_reply")
            || sceneId == QStringLiteral("scene10_arrive")) {
            if (gameState.stringValue(QStringLiteral("rain_song_emotion")) == QString::fromUtf8(u8"家庭")) {
                return QStringLiteral("bgm_dont_cry");
            }
            return QStringLiteral("bgm_meiyoulixiang");
        }
        if (stayedInClassroomRoute) {
            return QStringLiteral("bgm_chunfeng_chenzui");
        }
        return QStringLiteral("bgm_woxiang");
    }

    if (sceneId.startsWith(QStringLiteral("scene11_"))
        || sceneId.startsWith(QStringLiteral("scene12_"))) {
        if (stayedInClassroomRoute) {
            return QStringLiteral("bgm_chunfeng_chenzui");
        }
        return QStringLiteral("bgm_life_is");
    }

    return {};
}

bool shouldDelayBgmStart(const QString &sceneId)
{
    if (sceneId.startsWith(QStringLiteral("scene4_"))
        || sceneId.startsWith(QStringLiteral("scene10_"))
        || sceneId.startsWith(QStringLiteral("scene11_"))
        || sceneId == QStringLiteral("scene9_mainline_end")
        || sceneId == QStringLiteral("scene7a_hug")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3")
        || sceneId == QStringLiteral("scene10_song_performance")
        || sceneId == QStringLiteral("scene10_song_release")
        || sceneId == QStringLiteral("scene10_reply")
        || sceneId == QStringLiteral("scene10_arrive")) {
        return false;
    }

    return true;
}

bool shouldTypewriteCenterText(const QString &sceneId)
{
    return sceneId == QStringLiteral("scene8_breakdown_blackout")
        || sceneId == QStringLiteral("scene9_mainline_end")
        || sceneId == QStringLiteral("scene10_song_release")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3");
}

bool shouldKeepAmbientUnderBgm(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("scene3_"))
        || sceneId.startsWith(QStringLiteral("scene10_"))
        || sceneId.startsWith(QStringLiteral("scene11_"));
}

int ambientLeadOutDelayMsForScene(const QString &sceneId)
{
    if (shouldKeepAmbientUnderBgm(sceneId)) {
        return 0;
    }

    if (sceneId.startsWith(QStringLiteral("scene4_"))) {
        return 6000;
    }

    return 1000;
}

QString sceneEntryUiAudioId(const QString &sceneId)
{
    if (sceneId == QStringLiteral("scene11_message")
        || sceneId == QStringLiteral("scene11_message_2")
        || sceneId == QStringLiteral("scene11_message_3")
        || sceneId == QStringLiteral("scene12_message_notifications")
        || sceneId == QStringLiteral("scene12_message_notifications_2")
        || sceneId == QStringLiteral("scene12_message_notifications_3")
        || sceneId == QStringLiteral("scene12_ding_1")
        || sceneId == QStringLiteral("scene12_ding_2")) {
        return QStringLiteral("ui_phone_message_ping");
    }

    return {};
}

QString sceneEntryEmoAudioId(const QString &sceneId)
{
    if (sceneId == QStringLiteral("scene4_score_fall")) {
        return QStringLiteral("emo_score_low_rumble");
    }
    if (sceneId == QStringLiteral("scene8_teacher_first_sting")
        || sceneId == QStringLiteral("scene8_after_sting_choice")) {
        return QStringLiteral("emo_muffled_ring");
    }

    return {};
}

QString sceneEntrySfxAudioId(const QString &sceneId)
{
    if (sceneId == QStringLiteral("scene8_enter_prompt")) {
        return QStringLiteral("sfx_sink_water");
    }
    if (sceneId == QStringLiteral("scene9_bell")) {
        return QStringLiteral("sfx_bell");
    }

    return {};
}

} // namespace

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
      m_backgroundWidget(nullptr),
      m_shaderWidget(nullptr),
      m_characterPortraitLabel(nullptr),
      m_dialoguePanel(nullptr),
      m_centerTextLabel(nullptr),
      m_headerLabel(nullptr),
      m_audioManager(new AudioManager(this)),
      m_audioTestDialog(nullptr),
      m_narrativeEngine(new NarrativeEngine(this)),
      m_autoAdvanceTimer(new QTimer(this)),
      m_bgmStartDelayTimer(new QTimer(this)),
      m_ambStopAfterBgmTimer(new QTimer(this)),
      m_centerTextTypewriterTimer(new QTimer(this)),
      m_messageNotificationTimer(new QTimer(this)),
      m_linglingHugPortraitTimer(new QTimer(this)),
      m_textBlipThrottleTimer(new QElapsedTimer()),
      m_lastNarrativeViewState(nullptr),
      m_currentTextFrameIndex(-1),
      m_centerTextVisibleCharacters(0),
      m_messageNotificationIndex(0),
      m_centerTextFrameMode(false),
      m_scene8EnterPromptExpanded(false),
      m_linglingHugPortraitCompleted(false),
      m_messageNotificationAwaitingContinue(false),
      m_lastTextBlipMs(-1000)
{
    buildUi();

    connect(m_narrativeEngine, &NarrativeEngine::stateChanged, this, &GameWindow::applyNarrativeState);
    connect(m_autoAdvanceTimer, &QTimer::timeout, this, &GameWindow::advanceAutoNarrative);
    m_autoAdvanceTimer->setSingleShot(true);
    m_bgmStartDelayTimer->setSingleShot(true);
    connect(m_bgmStartDelayTimer, &QTimer::timeout, this, &GameWindow::startPendingBgm);
    m_ambStopAfterBgmTimer->setSingleShot(true);
    connect(m_ambStopAfterBgmTimer, &QTimer::timeout, this, &GameWindow::stopAmbientAfterBgmLeadIn);
    m_centerTextTypewriterTimer->setSingleShot(false);
    connect(m_centerTextTypewriterTimer, &QTimer::timeout, this, &GameWindow::advanceCenterTextTypewriter);
    m_messageNotificationTimer->setSingleShot(true);
    connect(m_messageNotificationTimer, &QTimer::timeout, this, &GameWindow::revealNextMessageNotification);
    m_linglingHugPortraitTimer->setSingleShot(true);
    connect(m_linglingHugPortraitTimer, &QTimer::timeout, this, &GameWindow::completeLinglingHugPortrait);
    m_textBlipThrottleTimer->start();

    loadAudioManifest();
    loadStoryContent();
    m_narrativeEngine->start();
}

void GameWindow::setSpeaker(const QString &speaker)
{
    m_dialoguePanel->setSpeaker(speaker);
}

void GameWindow::clearSpeaker()
{
    m_dialoguePanel->clearSpeaker();
}

void GameWindow::setText(const QString &text)
{
    m_dialoguePanel->setText(text);
    playTextBlipIfNeeded(text);
}

void GameWindow::setContinueVisible(bool visible)
{
    m_dialoguePanel->setContinueVisible(visible);
}

void GameWindow::setDialogueVisible(bool visible)
{
    m_dialoguePanel->setDialogueVisible(visible);
}

void GameWindow::setBackgroundStyle(BackgroundStyle style)
{
    m_backgroundWidget->setBackgroundStyle(style);

    const QPixmap pixmap = resolveBackgroundPixmap(style);
    if (pixmap.isNull()) {
        m_backgroundWidget->clearBackgroundPixmap();
        return;
    }

    m_backgroundWidget->setBackgroundPixmap(pixmap);
}

void GameWindow::setInteractionItems(const InteractionItems &items)
{
    m_dialoguePanel->setInteractionItems(items);
}

void GameWindow::setInteractionMode(InteractionMode mode)
{
    m_dialoguePanel->setInteractionMode(mode);
}

void GameWindow::showNarrationMode(const QString &text, bool showContinue)
{
    clearCenterText();
    setDialogueVisible(true);
    clearSpeaker();
    updateDialoguePanelBounds(false);
    setPagedText(text, showContinue);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
}

void GameWindow::showDialogueMode(const QString &speaker, const QString &text, bool showContinue)
{
    clearCenterText();
    setDialogueVisible(true);
    setSpeaker(speaker);
    updateDialoguePanelBounds(false);
    setPagedText(text, showContinue);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
}

void GameWindow::showPerformanceMode(const QString &text)
{
    if (isMessageNotificationScene(m_activeSceneId)) {
        showMessageNotificationMode(text);
        return;
    }

    resetMessageNotificationOverlay();
    setDialogueVisible(false);
    clearPagedText();
    updateDialoguePanelBounds(false);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
    setContinueVisible(false);
    m_sourceText = text;
    if (text.trimmed().isEmpty()) {
        m_centerTextFrameMode = false;
        m_currentTextFrameIndex = -1;
        stopCenterTextTypewriter();
        clearCenterText();
        return;
    }
    m_textFrames = buildCustomTextFrames(text);
    if (!m_textFrames.isEmpty()) {
        m_centerTextFrameMode = true;
        m_currentTextFrameIndex = 0;
        if (shouldTypewriteCenterText(m_activeSceneId)) {
            startCenterTextTypewriter(m_textFrames.first(), 58);
        } else {
            stopCenterTextTypewriter();
            setCenterText(m_textFrames.first());
        }
    } else if (shouldTypewriteCenterText(m_activeSceneId)) {
        m_centerTextFrameMode = false;
        m_currentTextFrameIndex = -1;
        startCenterTextTypewriter(text, 58);
    } else {
        m_centerTextFrameMode = false;
        m_currentTextFrameIndex = -1;
        stopCenterTextTypewriter();
        setCenterText(text);
    }
}

void GameWindow::showInteractionMode(const QString &speaker,
                                     const QString &text,
                                     InteractionMode mode,
                                     const InteractionItems &items,
                                     bool showContinue)
{
    clearCenterText();
    setDialogueVisible(true);
    clearPagedText();
    updateDialoguePanelBounds(!items.isEmpty());

    if (speaker.trimmed().isEmpty()) {
        clearSpeaker();
    } else {
        setSpeaker(speaker);
    }

    const QStringList customFrames = buildCustomTextFrames(text);
    if (!customFrames.isEmpty()) {
        m_sourceText = text;
        m_textFrames = customFrames;
        m_currentTextFrameIndex = 0;
        setText(m_textFrames.first());
        setInteractionMode(mode);
        setInteractionItems(items);
        m_dialoguePanel->setInteractionVisible(false);
        setContinueVisible(true);
        return;
    }

    setText(text);
    setInteractionMode(mode);
    setInteractionItems(items);
    m_dialoguePanel->setInteractionVisible(!items.isEmpty());
    setContinueVisible(showContinue);

    if (!items.isEmpty()) {
        m_dialoguePanel->focusFirstInteraction();
    }
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if ((event->key() == Qt::Key_Return
         || event->key() == Qt::Key_Enter
         || event->key() == Qt::Key_Space)
        && m_messageNotificationAwaitingContinue) {
        advanceMessageNotificationScene();
        event->accept();
        return;
    }

    if ((event->key() == Qt::Key_Return
         || event->key() == Qt::Key_Enter
         || event->key() == Qt::Key_Space)
        && m_centerTextFrameMode) {
        advanceNarrative();
        event->accept();
        return;
    }

    if (event->key() == Qt::Key_F9) {
        openAudioTestDialog();
        event->accept();
        return;
    }

    if ((event->key() == Qt::Key_Return
         || event->key() == Qt::Key_Enter
         || event->key() == Qt::Key_Space)
        && m_dialoguePanel->isContinueVisible()) {
        advanceNarrative();
        event->accept();
        return;
    }

    QMainWindow::keyPressEvent(event);
}

void GameWindow::resizeEvent(QResizeEvent *event)
{
    QMainWindow::resizeEvent(event);

    if (m_shaderWidget && m_backgroundWidget) {
        m_shaderWidget->setGeometry(m_backgroundWidget->rect());
    }

    updateMessageNotificationGeometry();
    updateDialoguePanelBounds(m_dialoguePanel && m_dialoguePanel->hasVisibleInteractions());
    refreshPagedText();
    if (m_characterPortraitLabel) {
        const QPixmap portrait = m_characterPortraitLabel->pixmap();
        if (!portrait.isNull()) {
            updateCharacterPortraitGeometry(portrait);
        }
    }
}

void GameWindow::advanceNarrative()
{
    if (m_centerTextFrameMode) {
        if (showNextTextFrame()) {
            return;
        }

        m_narrativeEngine->continueNarrative();
        return;
    }

    if (!m_dialoguePanel->isContinueVisible()) {
        return;
    }

    if (showNextTextFrame()) {
        const bool lastFrameReached = m_currentTextFrameIndex == m_textFrames.size() - 1;
        const bool shouldRevealInteractions = m_lastNarrativeViewState
            && m_lastNarrativeViewState->displayMode == NarrativeDisplayMode::Interaction
            && !m_lastNarrativeViewState->interactionItems.isEmpty()
            && lastFrameReached;
        if (shouldRevealInteractions) {
            if (m_activeSceneId == QStringLiteral("scene8_enter_prompt")) {
                m_scene8EnterPromptExpanded = true;
            }
            m_dialoguePanel->setInteractionVisible(true);
            m_dialoguePanel->focusFirstInteraction();
            setContinueVisible(false);
        }
        return;
    }

    m_narrativeEngine->continueNarrative();
}

void GameWindow::advanceAutoNarrative()
{
    const bool hasMoreFrames = !m_textFrames.isEmpty()
        && m_currentTextFrameIndex + 1 < m_textFrames.size();
    const bool shouldAdvanceFramesFirst = hasMoreFrames
        && (m_centerTextFrameMode || !m_dialoguePanel->isContinueVisible());
    if (shouldAdvanceFramesFirst) {
        showNextTextFrame();
        if (m_lastNarrativeViewState) {
            scheduleAutoAdvance(*m_lastNarrativeViewState);
        }
        return;
    }

    m_narrativeEngine->continueNarrative();
}

void GameWindow::advanceCenterTextTypewriter()
{
    if (!m_centerTextTypewriterTimer || m_centerTextTarget.isEmpty()) {
        return;
    }

    if (m_centerTextVisibleCharacters >= m_centerTextTarget.size()) {
        m_centerTextTypewriterTimer->stop();
        return;
    }

    ++m_centerTextVisibleCharacters;
    setCenterText(m_centerTextTarget.left(m_centerTextVisibleCharacters));
    if (m_centerTextVisibleCharacters >= m_centerTextTarget.size()) {
        m_centerTextTypewriterTimer->stop();
    }
}

void GameWindow::handleInteractionTriggered(const QString &id)
{
    if (!m_dialoguePanel->hasVisibleInteractions()) {
        return;
    }

    m_narrativeEngine->handleInteraction(id);
}

void GameWindow::handleContinueButtonClicked()
{
    playUiClickSound();
}

void GameWindow::handleInteractionButtonClicked(const QString &id)
{
    Q_UNUSED(id);
    playUiClickSound();
}

void GameWindow::applyNarrativeState()
{
    m_autoAdvanceTimer->stop();
    const QString previousSceneId = m_activeSceneId;

    const NarrativeViewState state = m_narrativeEngine->currentViewState();
    m_activeSceneId = state.sceneId;
    m_scene8EnterPromptExpanded = false;
    m_linglingHugPortraitCompleted = state.sceneId == QStringLiteral("scene7a_comfort")
        || state.sceneId == QStringLiteral("scene7a_comfort_2")
        || state.sceneId == QStringLiteral("scene7a_comfort_3");
    m_linglingHugPortraitTimer->stop();
    if (m_lastNarrativeViewState) {
        *m_lastNarrativeViewState = state;
    } else {
        m_lastNarrativeViewState = new NarrativeViewState(state);
    }

    if (!isMessageNotificationScene(state.sceneId)) {
        resetMessageNotificationOverlay();
    }

    const bool sceneChanged = previousSceneId != state.sceneId;
    syncAmbientAudio(state);
    syncBgmAudio(state, sceneChanged);
    playConfiguredSceneEntrySounds(state, sceneChanged);
    setHeaderText(state.header);
    setBackgroundStyle(state.backgroundStyle);
    setShaderEffect(state.shaderEffect);
    updateCharacterPortrait(state);

    if (state.displayMode == NarrativeDisplayMode::Interaction && !state.interactionItems.isEmpty()) {
        showInteractionMode(state.showSpeaker ? state.speaker : QString(),
                            state.text,
                            state.interactionMode,
                            state.interactionItems,
                            state.showContinue);
        if (state.autoAdvance) {
            scheduleAutoAdvance(state);
        }
        return;
    }

    if (state.showSpeaker) {
        showDialogueMode(state.speaker, state.text, state.showContinue);
    } else if (state.displayMode == NarrativeDisplayMode::Performance) {
        showPerformanceMode(state.text);
    } else {
        showNarrationMode(state.text, state.showContinue);
    }

    if (state.autoAdvance) {
        scheduleAutoAdvance(state);
    }
}

void GameWindow::openAudioTestDialog()
{
    if (!m_audioTestDialog) {
        m_audioTestDialog = new AudioTestDialog(m_audioManager, this);
    }

    m_audioTestDialog->show();
    m_audioTestDialog->raise();
    m_audioTestDialog->activateWindow();
}

void GameWindow::completeLinglingHugPortrait()
{
    m_linglingHugPortraitCompleted = true;
    if (!m_lastNarrativeViewState) {
        return;
    }

    updateCharacterPortrait(*m_lastNarrativeViewState);
}

void GameWindow::startPendingBgm()
{
    if (!m_audioManager || m_pendingBgmAudioId.isEmpty()) {
        return;
    }

    m_audioManager->playBgm(m_pendingBgmAudioId, 800, 800);
    const int ambLeadOutDelayMs = ambientLeadOutDelayMsForScene(m_activeSceneId);
    if (ambLeadOutDelayMs > 0 && !m_audioManager->currentAmbId().isEmpty()) {
        m_ambStopAfterBgmTimer->start(ambLeadOutDelayMs);
    }
    if (m_activeSceneId.startsWith(QStringLiteral("scene4_"))) {
        m_audioManager->duckBgm(0.18, 500);
    } else {
        m_audioManager->restoreBgm(500);
    }
    m_pendingBgmAudioId.clear();
}

void GameWindow::stopAmbientAfterBgmLeadIn()
{
    if (!m_audioManager || m_audioManager->currentBgmId().isEmpty()) {
        return;
    }

    if (m_audioManager->currentAmbId().isEmpty()) {
        return;
    }

    m_audioManager->stopAmb(500);
}

void GameWindow::buildUi()
{
    resize(1280, 720);
    setMinimumSize(960, 600);
    setWindowTitle(QString::fromUtf8(u8"第一天 - 最小数据驱动原型"));

    auto *audioTestAction = new QAction(QString::fromUtf8(u8"音频测试"), this);
    audioTestAction->setShortcut(QKeySequence(Qt::Key_F9));
    connect(audioTestAction, &QAction::triggered, this, &GameWindow::openAudioTestDialog);
    menuBar()->addAction(audioTestAction);

    m_backgroundWidget = new BackgroundWidget(this);
    setCentralWidget(m_backgroundWidget);

    auto *layout = new QVBoxLayout(m_backgroundWidget);
    layout->setContentsMargins(18, 14, 18, 14);
    layout->setSpacing(12);

    m_headerLabel = new QLabel(m_backgroundWidget);
    m_headerLabel->setStyleSheet(
        "QLabel {"
        "    color: rgba(120, 83, 112, 210);"
        "    background-color: rgba(255, 246, 251, 178);"
        "    border: 1px solid rgba(255, 255, 255, 205);"
        "    border-radius: 12px;"
        "    padding: 5px 12px;"
        "    font-size: 13px;"
        "    letter-spacing: 1px;"
        "    font-weight: 600;"
        "}"
    );
    layout->addWidget(m_headerLabel, 0, Qt::AlignLeft | Qt::AlignTop);

    layout->addStretch();

    m_centerTextLabel = new QLabel(m_backgroundWidget);
    m_centerTextLabel->setObjectName("centerTextLabel");
    m_centerTextLabel->setAlignment(Qt::AlignCenter);
    m_centerTextLabel->setWordWrap(true);
    m_centerTextLabel->setMinimumHeight(120);
    m_centerTextLabel->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    m_centerTextLabel->setStyleSheet(
        "QLabel#centerTextLabel {"
        "    color: rgb(255, 247, 251);"
        "    font-size: 24px;"
        "    line-height: 145%;"
        "    font-weight: 500;"
        "    background: transparent;"
        "}"
    );
    m_centerTextLabel->hide();
    layout->addWidget(m_centerTextLabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addStretch();

    m_characterPortraitLabel = new QLabel(m_backgroundWidget);
    m_characterPortraitLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_characterPortraitLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    m_characterPortraitLabel->setScaledContents(true);
    m_characterPortraitLabel->hide();

    m_dialoguePanel = new DialoguePanel(m_backgroundWidget);
    updateDialoguePanelBounds(false);
    m_dialoguePanel->show();

    m_messageNotificationOverlay = new QWidget(m_backgroundWidget);
    m_messageNotificationOverlay->setAttribute(Qt::WA_TransparentForMouseEvents, false);
    m_messageNotificationOverlay->setStyleSheet(QStringLiteral("background: rgba(8, 11, 18, 28);"));
    m_messageNotificationOverlay->hide();

    auto *overlayLayout = new QVBoxLayout(m_messageNotificationOverlay);
    overlayLayout->setContentsMargins(26, 24, 0, 0);

    m_messageNotificationCard = new QWidget(m_messageNotificationOverlay);
    m_messageNotificationCard->setStyleSheet(QStringLiteral("background: transparent;"));
    m_messageNotificationCard->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Maximum);
    auto *cardLayout = new QVBoxLayout(m_messageNotificationCard);
    cardLayout->setContentsMargins(0, 0, 0, 0);
    cardLayout->setSpacing(12);
    cardLayout->setAlignment(Qt::AlignTop | Qt::AlignLeft);

    m_messageNotificationMessagesWidget = new QWidget(m_messageNotificationCard);
    m_messageNotificationMessagesWidget->setStyleSheet(QStringLiteral("background: transparent; border: none;"));
    m_messageNotificationMessagesWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    cardLayout->addWidget(m_messageNotificationMessagesWidget, 0, Qt::AlignLeft);

    m_messageNotificationContinueButton = new QPushButton(QString::fromUtf8(u8"继续"), m_messageNotificationCard);
    m_messageNotificationContinueButton->setCursor(Qt::PointingHandCursor);
    m_messageNotificationContinueButton->setStyleSheet(
        "QPushButton {"
        "    min-width: 78px;"
        "    padding: 7px 16px;"
        "    border-radius: 18px;"
        "    color: rgb(244, 247, 255);"
        "    background: rgba(20, 24, 34, 120);"
        "    border: 1px solid rgba(255, 255, 255, 30);"
        "}"
        "QPushButton:hover {"
        "    background: rgba(28, 33, 46, 144);"
        "}"
        "QPushButton:pressed {"
        "    background: rgba(38, 44, 60, 164);"
        "}"
    );
    m_messageNotificationContinueButton->hide();
    connect(m_messageNotificationContinueButton, &QPushButton::clicked, this, &GameWindow::advanceMessageNotificationScene);
    cardLayout->addWidget(m_messageNotificationContinueButton, 0, Qt::AlignLeft);

    overlayLayout->addWidget(m_messageNotificationCard, 0, Qt::AlignLeft | Qt::AlignTop);
    overlayLayout->addStretch();

    m_shaderWidget = new ShaderToyWidget(m_backgroundWidget);
    m_shaderWidget->setGeometry(m_backgroundWidget->rect());
    m_shaderWidget->hide();
    m_shaderWidget->raise();

    m_headerLabel->raise();
    m_centerTextLabel->raise();
    m_dialoguePanel->raise();
    m_messageNotificationOverlay->raise();

    connect(m_dialoguePanel, &DialoguePanel::continueRequested, this, &GameWindow::advanceNarrative);
    connect(m_dialoguePanel, &DialoguePanel::continueButtonClicked, this, &GameWindow::handleContinueButtonClicked);
    connect(m_dialoguePanel, &DialoguePanel::interactionTriggered, this, &GameWindow::handleInteractionTriggered);
    connect(m_dialoguePanel, &DialoguePanel::interactionButtonClicked, this, &GameWindow::handleInteractionButtonClicked);
}

void GameWindow::loadAudioManifest()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidatePaths = {
        QDir(appDir).filePath(QStringLiteral("data/audio_manifest.json")),
        QDir(appDir).filePath(QStringLiteral("../data/audio_manifest.json"))
    };

    for (const QString &candidatePath : candidatePaths) {
        if (!QFileInfo::exists(candidatePath)) {
            continue;
        }

        if (m_audioManager->loadManifest(candidatePath)) {
            return;
        }
    }

    qWarning().noquote() << QStringLiteral("[AudioManager] No audio manifest loaded.");
}

void GameWindow::loadStoryContent()
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidatePaths = {
        QDir(appDir).filePath(QStringLiteral("data/story_main.json")),
        QDir(appDir).filePath(QStringLiteral("../data/story_main.json"))
    };

    for (const QString &candidatePath : candidatePaths) {
        const StoryLoadResult result = StoryLoader::loadFromFile(candidatePath);
        if (!result.success) {
            qWarning().noquote() << QStringLiteral("[StoryLoader] Failed to load %1: %2")
                                    .arg(candidatePath, result.errorMessage);
            continue;
        }

        qInfo().noquote() << QStringLiteral("[StoryLoader] Loaded %1 (%2 scenes, %3 interactions)")
                              .arg(result.filePath)
                              .arg(result.sceneCount)
                              .arg(result.interactionCount);

        m_narrativeEngine->loadScenes(result.scenes, result.startSceneId);
        return;
    }

    qWarning().noquote() << QStringLiteral("[StoryLoader] Falling back to built-in sample story.");
    m_narrativeEngine->loadScenes(createSampleStory(), sampleStoryStartSceneId());
}

void GameWindow::syncAmbientAudio(const NarrativeViewState &state)
{
    if (!m_audioManager) {
        return;
    }

    QString audioId = ambientAudioIdForScene(state.sceneId);
    if ((state.sceneId.startsWith(QStringLiteral("scene1_"))
         || state.sceneId.startsWith(QStringLiteral("scene2_")))
        && state.speaker == QString::fromUtf8(u8"川哥")
        && state.sceneId != QStringLiteral("scene2_report_stuck")) {
        audioId.clear();
    }

    if (shouldSuppressAmbientForState(state)) {
        if (!m_audioManager->currentAmbId().isEmpty()) {
            m_ambStopAfterBgmTimer->stop();
            m_audioManager->stopAmb(500);
        }
        return;
    }

    if (audioId.isEmpty()) {
        m_ambStopAfterBgmTimer->stop();
        m_audioManager->stopAmb(800);
        return;
    }

    m_audioManager->playAmb(audioId, 800, 800);
}

void GameWindow::syncBgmAudio(const NarrativeViewState &state, bool sceneChanged)
{
    if (!m_audioManager) {
        return;
    }

    const QString audioId = bgmAudioIdForScene(state, m_narrativeEngine->gameState());
    const QString currentBgmId = m_audioManager->currentBgmId();
    const bool switchingToDifferentBgm = !currentBgmId.isEmpty() && currentBgmId != audioId;
    const int sceneChangeFadeOutMs = sceneChanged && switchingToDifferentBgm ? 0 : 800;
    if (audioId.isEmpty()) {
        m_pendingBgmAudioId.clear();
        m_bgmStartDelayTimer->stop();
        m_ambStopAfterBgmTimer->stop();
        m_audioManager->stopBgm(sceneChanged ? 0 : 800);
        return;
    }

    if (currentBgmId == audioId && m_pendingBgmAudioId.isEmpty()) {
        const int ambLeadOutDelayMs = ambientLeadOutDelayMsForScene(state.sceneId);
        if (sceneChanged && ambLeadOutDelayMs > 0 && !m_audioManager->currentAmbId().isEmpty()) {
            m_ambStopAfterBgmTimer->start(ambLeadOutDelayMs);
        }
        if (state.sceneId.startsWith(QStringLiteral("scene4_"))) {
            m_audioManager->duckBgm(0.18, 500);
        } else {
            m_audioManager->restoreBgm(500);
        }
        return;
    }

    if (!sceneChanged && m_pendingBgmAudioId == audioId) {
        return;
    }

    if (!shouldDelayBgmStart(state.sceneId)) {
        m_pendingBgmAudioId.clear();
        m_bgmStartDelayTimer->stop();
        m_audioManager->playBgm(audioId, sceneChangeFadeOutMs, 800);
        const int ambLeadOutDelayMs = ambientLeadOutDelayMsForScene(state.sceneId);
        if (ambLeadOutDelayMs > 0 && !m_audioManager->currentAmbId().isEmpty()) {
            m_ambStopAfterBgmTimer->start(ambLeadOutDelayMs);
        }
        if (state.sceneId.startsWith(QStringLiteral("scene4_"))) {
            m_audioManager->duckBgm(0.18, 500);
        } else {
            m_audioManager->restoreBgm(500);
        }
        return;
    }

    if (m_pendingBgmAudioId == audioId && m_bgmStartDelayTimer->isActive()) {
        return;
    }

    m_pendingBgmAudioId = audioId;
    m_ambStopAfterBgmTimer->stop();
    m_bgmStartDelayTimer->start(3000);
    if (!currentBgmId.isEmpty() && currentBgmId != audioId) {
        m_audioManager->stopBgm(sceneChanged ? 0 : 800);
    } else if (state.sceneId.startsWith(QStringLiteral("scene4_"))) {
        m_audioManager->duckBgm(0.18, 500);
    }
}

bool GameWindow::shouldSuppressAmbientForState(const NarrativeViewState &state) const
{
    if (!m_audioManager || !m_narrativeEngine) {
        return false;
    }

    const QString desiredBgmId = bgmAudioIdForScene(state, m_narrativeEngine->gameState());
    if (desiredBgmId.isEmpty()) {
        return false;
    }

    if (shouldKeepAmbientUnderBgm(state.sceneId)) {
        return false;
    }

    if (!m_pendingBgmAudioId.isEmpty()) {
        return false;
    }

    if (m_ambStopAfterBgmTimer && m_ambStopAfterBgmTimer->isActive()) {
        return false;
    }

    return m_audioManager->currentBgmId() == desiredBgmId
        && m_audioManager->currentAmbId().isEmpty();
}

void GameWindow::playConfiguredSceneEntrySounds(const NarrativeViewState &state, bool sceneChanged)
{
    if (!sceneChanged || !m_audioManager) {
        return;
    }

    if (state.sceneId == QStringLiteral("scene8_score_question")) {
        m_audioManager->stopSfx(QStringLiteral("sfx_sink_water"));
    }

    const QString uiAudioId = sceneEntryUiAudioId(state.sceneId);
    if (!uiAudioId.isEmpty()) {
        m_audioManager->playUi(uiAudioId);
    }

    const QString emoAudioId = sceneEntryEmoAudioId(state.sceneId);
    if (!emoAudioId.isEmpty()) {
        m_audioManager->playEmo(emoAudioId);
    }

    const QString sfxAudioId = sceneEntrySfxAudioId(state.sceneId);
    if (!sfxAudioId.isEmpty()) {
        m_audioManager->playSfx(sfxAudioId);
    }
}

void GameWindow::playTextBlipIfNeeded(const QString &text)
{
    if (!m_audioManager || text.trimmed().isEmpty() || !m_textBlipThrottleTimer) {
        return;
    }

    const qint64 nowMs = m_textBlipThrottleTimer->elapsed();
    if (nowMs - m_lastTextBlipMs < 120) {
        return;
    }

    m_lastTextBlipMs = nowMs;
    m_audioManager->playUi(QStringLiteral("ui_text_blip_soft"));
}

void GameWindow::playUiClickSound()
{
    if (!m_audioManager) {
        return;
    }

    m_audioManager->playUi(QStringLiteral("ui_click_soft"));
}

bool GameWindow::isMessageNotificationScene(const QString &sceneId) const
{
    Q_UNUSED(sceneId);
    return false;
}

void GameWindow::showMessageNotificationMode(const QString &text)
{
    setDialogueVisible(false);
    clearPagedText();
    updateDialoguePanelBounds(false);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
    setContinueVisible(false);
    clearCenterText();

    const QString normalizedText = text.trimmed();
    if (m_messageNotificationOverlay->isVisible()
        && m_messageNotificationTexts.join(QStringLiteral("\n\n")) == normalizedText) {
        return;
    }

    resetMessageNotificationOverlay();
    m_messageNotificationTexts = normalizedText.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
    for (QString &message : m_messageNotificationTexts) {
        message = message.trimmed();
    }
    m_messageNotificationIndex = 0;
    m_messageNotificationAwaitingContinue = false;
    updateMessageNotificationGeometry();
    layoutMessageNotificationRows();
    m_messageNotificationOverlay->show();
    m_messageNotificationOverlay->raise();
    m_messageNotificationCard->show();
    revealNextMessageNotification();
}

void GameWindow::resetMessageNotificationOverlay()
{
    if (m_messageNotificationTimer) {
        m_messageNotificationTimer->stop();
    }
    m_messageNotificationTexts.clear();
    m_messageNotificationIndex = 0;
    m_messageNotificationAwaitingContinue = false;
    m_messageNotificationContinueButton->hide();
    if (m_messageNotificationMessagesWidget) {
        m_messageNotificationMessagesWidget->hide();
        m_messageNotificationMessagesWidget->resize(0, 0);
    }
    for (QWidget *widget : std::as_const(m_messageNotificationBubbles)) {
        if (widget) {
            delete widget;
        }
    }
    m_messageNotificationBubbles.clear();
    m_messageNotificationOverlay->hide();
}

void GameWindow::layoutMessageNotificationRows()
{
    if (!m_messageNotificationMessagesWidget) {
        return;
    }

    for (QWidget *row : std::as_const(m_messageNotificationBubbles)) {
        if (!row) {
            continue;
        }
        delete row;
    }
    m_messageNotificationBubbles.clear();

    if (m_messageNotificationTexts.isEmpty()) {
        m_messageNotificationMessagesWidget->hide();
        m_messageNotificationMessagesWidget->resize(0, 0);
        return;
    }

    const int bubbleX = 70;
    const int gapY = 18;
    const int bubbleMaxWidth = qMax(220, m_messageNotificationCard->width() - 118);
    int y = 0;
    int maxWidth = 0;

    for (int rowIndex = 0; rowIndex < m_messageNotificationTexts.size(); ++rowIndex) {
        const bool isLastMessage = rowIndex == m_messageNotificationTexts.size() - 1;
        auto *row = new QWidget(m_messageNotificationMessagesWidget);
        row->setStyleSheet(QStringLiteral("background: transparent;"));
        row->setAttribute(Qt::WA_StyledBackground, true);
        row->hide();

        if (rowIndex == 0) {
            auto *avatar = new QLabel(QString::fromUtf8(u8"灵"), row);
            avatar->setAlignment(Qt::AlignCenter);
            avatar->setFixedSize(54, 54);
            avatar->move(0, 0);
            avatar->setStyleSheet(
                "QLabel {"
                "    color: rgb(32, 37, 46);"
                "    background: rgba(255, 255, 255, 244);"
                "    border-radius: 27px;"
                "    font-size: 20px;"
                "    font-weight: 700;"
                "}"
            );
        }

        auto *bubble = new QLabel(m_messageNotificationTexts.at(rowIndex), row);
        bubble->setWordWrap(true);
        bubble->setMinimumHeight(54);
        bubble->setMaximumWidth(bubbleMaxWidth);
        bubble->setStyleSheet(isLastMessage
            ? QStringLiteral(
                "QLabel {"
                "    color: rgb(24, 27, 34);"
                "    background: rgba(255, 255, 255, 252);"
                "    border: 1px solid rgba(255, 255, 255, 218);"
                "    border-radius: 22px;"
                "    padding: 14px 24px;"
                "    font-size: 18px;"
                "    font-weight: 600;"
                "    line-height: 138%;"
                "}"
            )
            : QStringLiteral(
                "QLabel {"
                "    color: rgb(24, 27, 34);"
                "    background: rgba(255, 255, 255, 244);"
                "    border: 1px solid rgba(255, 255, 255, 210);"
                "    border-radius: 22px;"
                "    padding: 14px 24px;"
                "    font-size: 18px;"
                "    line-height: 138%;"
                "}"
            ));
        bubble->adjustSize();
        bubble->move(bubbleX, 0);

        const int rowHeight = qMax(rowIndex == 0 ? 54 : 0, bubble->height());
        row->setGeometry(0, y, bubbleX + bubble->width(), rowHeight);
        if (rowIndex < m_messageNotificationIndex) {
            row->show();
        }

        m_messageNotificationBubbles.append(row);
        maxWidth = qMax(maxWidth, row->width());
        y += rowHeight + gapY;
    }

    const int contentHeight = y > 0 ? y - gapY : 0;
    m_messageNotificationMessagesWidget->resize(maxWidth, contentHeight);
    m_messageNotificationMessagesWidget->show();
    m_messageNotificationCard->adjustSize();
}

void GameWindow::revealNextMessageNotification()
{
    if (m_messageNotificationTexts.isEmpty()) {
        completeMessageNotificationSequence();
        return;
    }

    if (!m_messageNotificationOverlay->isVisible()) {
        m_messageNotificationOverlay->show();
        m_messageNotificationOverlay->raise();
        auto *cardEffect = new QGraphicsOpacityEffect(m_messageNotificationCard);
        m_messageNotificationCard->setGraphicsEffect(cardEffect);
        auto *cardFade = new QPropertyAnimation(cardEffect, "opacity", m_messageNotificationCard);
        cardFade->setDuration(220);
        cardFade->setStartValue(0.0);
        cardFade->setEndValue(1.0);
        cardFade->start(QAbstractAnimation::DeleteWhenStopped);
    }

    if (m_messageNotificationIndex >= m_messageNotificationTexts.size()) {
        completeMessageNotificationSequence();
        return;
    }

    QWidget *row = m_messageNotificationBubbles.value(m_messageNotificationIndex, nullptr);
    ++m_messageNotificationIndex;
    if (row) {
        auto *effect = new QGraphicsOpacityEffect(row);
        row->setGraphicsEffect(effect);
        effect->setOpacity(0.0);
        auto *fade = new QPropertyAnimation(effect, "opacity", row);
        fade->setDuration(320);
        fade->setStartValue(0.0);
        fade->setEndValue(1.0);
        fade->start(QAbstractAnimation::DeleteWhenStopped);
        row->show();
    }
    if (m_audioManager) {
        m_audioManager->playUi(QStringLiteral("ui_phone_message_ping"));
    }

    if (m_messageNotificationIndex < m_messageNotificationTexts.size()) {
        m_messageNotificationTimer->start(5000);
    } else {
        m_messageNotificationTimer->start(5000);
    }
}

void GameWindow::completeMessageNotificationSequence()
{
    m_messageNotificationAwaitingContinue = true;
    m_messageNotificationContinueButton->show();
    m_messageNotificationContinueButton->raise();
    m_messageNotificationOverlay->raise();
}

void GameWindow::advanceMessageNotificationScene()
{
    if (!m_messageNotificationAwaitingContinue || !m_narrativeEngine) {
        return;
    }

    playUiClickSound();
    resetMessageNotificationOverlay();
    m_narrativeEngine->continueNarrative();
}

void GameWindow::updateMessageNotificationGeometry()
{
    if (!m_backgroundWidget || !m_messageNotificationOverlay || !m_messageNotificationCard) {
        return;
    }

    const QRect rect = m_backgroundWidget->rect();
    m_messageNotificationOverlay->setGeometry(rect);
    const int cardWidth = qMin(520, qRound(rect.width() * 0.44));
    m_messageNotificationCard->setFixedWidth(qMax(340, cardWidth));
    if (!m_messageNotificationTexts.isEmpty()) {
        layoutMessageNotificationRows();
    }
}

void GameWindow::setHeaderText(const QString &text)
{
    m_headerLabel->setText(text);
}

void GameWindow::setCenterText(const QString &text)
{
    m_centerTextLabel->setText(text);
    m_centerTextLabel->setVisible(!text.trimmed().isEmpty());
}

void GameWindow::clearCenterText()
{
    stopCenterTextTypewriter();
    m_centerTextLabel->clear();
    m_centerTextLabel->hide();
}

void GameWindow::startCenterTextTypewriter(const QString &text, int intervalMs)
{
    stopCenterTextTypewriter();
    m_centerTextTarget = text;
    m_centerTextVisibleCharacters = 0;
    setCenterText(QString());
    if (m_centerTextTarget.isEmpty()) {
        return;
    }

    m_centerTextTypewriterTimer->start(qMax(20, intervalMs));
}

void GameWindow::stopCenterTextTypewriter()
{
    if (m_centerTextTypewriterTimer) {
        m_centerTextTypewriterTimer->stop();
    }
    m_centerTextTarget.clear();
    m_centerTextVisibleCharacters = 0;
}

void GameWindow::setPagedText(const QString &text, bool showContinue)
{
    m_sourceText = text;
    m_textFrames.clear();
    m_currentTextFrameIndex = -1;
    m_centerTextFrameMode = false;

    const QStringList customFrames = buildCustomTextFrames(text);
    if (!customFrames.isEmpty()) {
        m_textFrames = customFrames;
    }

    if (showContinue && m_textFrames.isEmpty()) {
        m_textFrames = m_dialoguePanel->paginateTextFrames(text, 1);
    }

    if (m_textFrames.isEmpty()) {
        setText(text);
        setContinueVisible(showContinue);
        return;
    }

    m_currentTextFrameIndex = 0;
    setText(m_textFrames.first());
    setContinueVisible(true);
}

bool GameWindow::showNextTextFrame()
{
    if (m_textFrames.isEmpty()) {
        return false;
    }

    if (m_currentTextFrameIndex + 1 >= m_textFrames.size()) {
        return false;
    }

    ++m_currentTextFrameIndex;
    const QString nextFrame = m_textFrames.at(m_currentTextFrameIndex);
    if (m_centerTextFrameMode) {
        if (shouldTypewriteCenterText(m_activeSceneId)) {
            startCenterTextTypewriter(nextFrame, 58);
        } else {
            stopCenterTextTypewriter();
            setCenterText(nextFrame);
        }
    } else {
        setText(nextFrame);
        setContinueVisible(true);
    }
    return true;
}

void GameWindow::clearPagedText()
{
    m_sourceText.clear();
    m_textFrames.clear();
    m_currentTextFrameIndex = -1;
    m_centerTextFrameMode = false;
}

void GameWindow::refreshPagedText()
{
    if (m_sourceText.trimmed().isEmpty() || m_textFrames.isEmpty()) {
        return;
    }

    const int oldFrameIndex = m_currentTextFrameIndex;
    const QStringList customFrames = buildCustomTextFrames(m_sourceText);
    m_textFrames = customFrames.isEmpty()
        ? m_dialoguePanel->paginateTextFrames(m_sourceText, 1)
        : customFrames;
    if (m_textFrames.isEmpty()) {
        m_currentTextFrameIndex = -1;
        setText(m_sourceText);
        return;
    }

    m_currentTextFrameIndex = qBound(0, oldFrameIndex, m_textFrames.size() - 1);
    if (m_centerTextFrameMode) {
        setCenterText(m_textFrames.at(m_currentTextFrameIndex));
    } else {
        setText(m_textFrames.at(m_currentTextFrameIndex));
    }
}

void GameWindow::updateDialoguePanelBounds(bool hasInteractions)
{
    if (!m_dialoguePanel || !m_backgroundWidget) {
        return;
    }

    const QRect rect = m_backgroundWidget->rect();
    const int panelWidth = hasInteractions
        ? qMax(1080, qRound(rect.width() * 0.82))
        : qMax(980, qRound(rect.width() * 0.70));
    const int panelHeight = hasInteractions ? 230 : 120;
    const int x = (rect.width() - panelWidth) / 2;
    const int y = rect.height() - panelHeight - 26;

    m_dialoguePanel->setGeometry(x, y, panelWidth, panelHeight);
    m_dialoguePanel->raise();
}

void GameWindow::updateCharacterPortrait(const NarrativeViewState &state)
{
    if (!m_characterPortraitLabel) {
        return;
    }

    const bool inEarlyScenes = state.sceneId.startsWith(QStringLiteral("scene1_"))
        || state.sceneId.startsWith(QStringLiteral("scene2_"))
        || state.sceneId.startsWith(QStringLiteral("scene3_"))
        || state.sceneId.startsWith(QStringLiteral("scene4_"))
        || state.sceneId.startsWith(QStringLiteral("scene5_"))
        || state.sceneId.startsWith(QStringLiteral("scene6_"));
    const bool inLinglingScene6 = state.sceneId == QStringLiteral("scene6_invitation")
        || state.sceneId == QStringLiteral("scene6_go_downstairs_hesitation");
    const bool inLinglingOpening = state.sceneId == QStringLiteral("scene7a_opening");
    const bool inLinglingTopics = state.sceneId == QStringLiteral("scene7a_topics");
    const bool inLinglingHug = state.sceneId == QStringLiteral("scene7a_hug");
    const bool inLinglingComfort = state.sceneId == QStringLiteral("scene7a_comfort")
        || state.sceneId == QStringLiteral("scene7a_comfort_2")
        || state.sceneId == QStringLiteral("scene7a_comfort_3");
    const bool inOfficeScene = state.sceneId.startsWith(QStringLiteral("scene8_"));
    const bool inOfficeDoorway = state.sceneId == QStringLiteral("scene8_enter_prompt");
    const bool inOfficeReturnMonologue = state.sceneId.startsWith(QStringLiteral("scene8_return_to_classroom"));
    const bool inOfficeBlackout = state.sceneId == QStringLiteral("scene8_breakdown_blackout")
        || state.sceneId == QStringLiteral("scene8_breakdown_ellipsis");
    const bool shouldShowChuanGe = (inEarlyScenes
            && state.showSpeaker
            && state.speaker == QString::fromUtf8(u8"川哥"))
        || (inOfficeScene && !inOfficeDoorway && !inOfficeReturnMonologue && !inOfficeBlackout);
    const bool shouldShowLingling = inLinglingScene6
        || inLinglingOpening
        || inLinglingTopics
        || inLinglingHug
        || inLinglingComfort;
    const bool shouldShow = state.shaderEffect == ShaderEffect::None
        && (shouldShowChuanGe || shouldShowLingling);

    if (!shouldShow) {
        m_characterPortraitLabel->hide();
        return;
    }

    QString portraitId;
    if (shouldShowLingling) {
        if (inLinglingScene6) {
            portraitId = QString::fromUtf8(u8"灵灵/站.png");
        } else if (inLinglingOpening) {
            portraitId = QString::fromUtf8(u8"灵灵/站听.png");
        } else if (inLinglingTopics) {
            const QString hidePlace = m_narrativeEngine->gameState().stringValue(QStringLiteral("hide_place"));
            portraitId = hidePlace == QStringLiteral("gym_back")
                ? QString::fromUtf8(u8"灵灵/坐听.png")
                : QString::fromUtf8(u8"灵灵/站听.png");
        } else if (inLinglingHug) {
            if (state.text.contains(QString::fromUtf8(u8"灵灵伸手"))
                || state.text.contains(QString::fromUtf8(u8"伸手"))) {
                if (!m_linglingHugPortraitCompleted) {
                    m_linglingHugPortraitTimer->start(1000);
                    portraitId = QString::fromUtf8(u8"灵灵/张开双手.png");
                } else {
                    portraitId = QString::fromUtf8(u8"灵灵/拥抱.png");
                }
            } else if (state.text.contains(QString::fromUtf8(u8"抱住你"))) {
                m_linglingHugPortraitCompleted = true;
                portraitId = QString::fromUtf8(u8"灵灵/拥抱.png");
            } else if (state.text.contains(QString::fromUtf8(u8"你终于彻底哭出来了"))) {
                m_linglingHugPortraitCompleted = true;
                portraitId = QString::fromUtf8(u8"灵灵/拥抱.png");
            } else {
                portraitId = QString::fromUtf8(u8"灵灵/站听.png");
            }
        } else {
            portraitId = QString::fromUtf8(u8"灵灵/拥抱.png");
        }
    } else if (inOfficeScene) {
        const bool isConflictSection = state.sceneId.startsWith(QStringLiteral("scene8_teacher_first_sting"))
            || state.sceneId.startsWith(QStringLiteral("scene8_after_sting_choice"))
            || state.sceneId.startsWith(QStringLiteral("scene8_explain_try"))
            || state.sceneId.startsWith(QStringLiteral("scene8_conflict_feedback_"));
        portraitId = isConflictSection
            ? QString::fromUtf8(u8"川哥/坐_angry.png")
            : QString::fromUtf8(u8"川哥/坐_peace.png");
    } else {
        portraitId = QString::fromUtf8(u8"川哥/站.png");
    }

    m_characterPortraitCache.remove(portraitId);
    const QPixmap portrait = resolveCharacterPortrait(portraitId);
    if (portrait.isNull()) {
        m_characterPortraitLabel->hide();
        return;
    }

    m_characterPortraitLabel->setPixmap(portrait);
    updateCharacterPortraitGeometry(portrait);
    m_characterPortraitLabel->show();
    m_characterPortraitLabel->raise();
    m_headerLabel->raise();
    m_centerTextLabel->raise();
    m_dialoguePanel->raise();
}

void GameWindow::updateCharacterPortraitGeometry(const QPixmap &portrait)
{
    if (!m_characterPortraitLabel || !m_backgroundWidget) {
        return;
    }

    const QRect rect = m_backgroundWidget->rect();
    if (rect.isEmpty()) {
        return;
    }

    if (portrait.isNull() || portrait.height() <= 0) {
        return;
    }

    const qreal portraitAspectRatio = static_cast<qreal>(portrait.width()) / portrait.height();
    const int portraitHeight = qRound(rect.height() * 0.56);
    const int portraitWidth = qRound(portraitHeight * portraitAspectRatio);
    const int x = -2;
    const int y = rect.bottom() - portraitHeight + 2;
    m_characterPortraitLabel->setGeometry(x, y, portraitWidth, portraitHeight);
}

void GameWindow::scheduleAutoAdvance(const NarrativeViewState &state)
{
    int durationMs = state.autoAdvanceDurationMs;
    if (durationMs <= 0) {
        durationMs = 900 + state.text.trimmed().size() * 85;
        durationMs = qMax(1100, qMin(durationMs, 4200));
    }
    if (state.sceneId == QStringLiteral("scene7a_hug")
        && state.text.contains(QString::fromUtf8(u8"灵灵伸手抱住你"))) {
        durationMs = qMax(durationMs, 1800);
    }
    if (state.sceneId == QStringLiteral("scene10_song_performance")) {
        if (state.text.contains(QString::fromUtf8(u8"然后你开始唱"))) {
            durationMs = 9000;
        } else {
            durationMs = 2000;
        }
    }
    if (state.sceneId == QStringLiteral("scene8_return_to_classroom")) {
        durationMs = 4000;
    }
    if (state.sceneId == QStringLiteral("scene9_mainline_end")) {
        durationMs += 1000;
        if (!m_textFrames.isEmpty() && m_currentTextFrameIndex == m_textFrames.size() - 1) {
            durationMs += 1000;
        }
    }
    m_autoAdvanceTimer->start(durationMs);
}

void GameWindow::setShaderEffect(ShaderEffect effect)
{
    if (!m_shaderWidget) {
        return;
    }

    if (effect == ShaderEffect::None) {
        m_shaderWidget->hide();
        m_shaderWidget->setShaderEffect(ShaderEffect::None);
        return;
    }

    m_shaderWidget->setShaderEffect(effect);
    m_shaderWidget->restartAnimation();
    m_shaderWidget->show();
    m_shaderWidget->raise();
}

QStringList GameWindow::buildCustomTextFrames(const QString &text) const
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));
    if (normalized.contains(QStringLiteral("\n\n"))) {
        QStringList explicitFrames = normalized.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts);
        for (QString &frame : explicitFrames) {
            frame = frame.trimmed();
        }
        explicitFrames.removeAll(QString());
        if (!explicitFrames.isEmpty()) {
            return explicitFrames;
        }
    }

    if (m_activeSceneId != QStringLiteral("scene8_enter_prompt")) {
        return {};
    }

    const QString marker = QString::fromUtf8(u8"。灵灵把你送到办公室门口");
    const int splitIndex = normalized.indexOf(marker);
    if (splitIndex < 0) {
        return {};
    }

    return {
        normalized.left(splitIndex + 1).trimmed(),
        normalized.mid(splitIndex + 1).trimmed()
    };
}

QPixmap GameWindow::resolveCharacterPortrait(const QString &characterId)
{
    auto it = m_characterPortraitCache.constFind(characterId);
    if (it != m_characterPortraitCache.cend()) {
        return it.value();
    }

    for (const QString &root : backgroundSearchRoots()) {
        const QString imagePath = QDir(root).filePath(characterId);
        if (!QFileInfo::exists(imagePath)) {
            continue;
        }

        QPixmap pixmap(imagePath);
        if (!pixmap.isNull()) {
            pixmap = trimTransparentMargins(pixmap);
            m_characterPortraitCache.insert(characterId, pixmap);
            return pixmap;
        }
    }

    m_characterPortraitCache.insert(characterId, QPixmap());
    return {};
}

QPixmap GameWindow::resolveBackgroundPixmap(BackgroundStyle style)
{
    const int cacheKey = static_cast<int>(style);
    if (m_backgroundCache.contains(cacheKey)) {
        return m_backgroundCache.value(cacheKey);
    }

    for (const QString &root : backgroundSearchRoots()) {
        for (const QString &imageName : imageNamesForBackground(style)) {
            const QString imagePath = QDir(root).filePath(imageName);
            if (!QFileInfo::exists(imagePath)) {
                continue;
            }

            QPixmap pixmap(imagePath);
            if (!pixmap.isNull()) {
                m_backgroundCache.insert(cacheKey, pixmap);
                return pixmap;
            }
        }
    }

    m_backgroundCache.insert(cacheKey, QPixmap());
    return {};
}

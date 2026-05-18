#include "gamewindow.h"

#include "audio/audiomanager.h"
#include "audio/audiotestdialog.h"
#include "narrative/samplestory.h"
#include "narrative/storyloader.h"
#include "ui/dialoguepanel.h"
#include "ui/floatingchoicelayer.h"
#include "ui/shadertoywidget.h"

#include <QAction>
#include <QCoreApplication>
#include <QDebug>
#include <QDir>
#include <QDirIterator>
#include <QEasingCurve>
#include <QElapsedTimer>
#include <QFileInfo>
#include <QImage>
#include <QKeyEvent>
#include <QLabel>
#include <QLayout>
#include <QMenuBar>
#include <QPixmap>
#include <QPushButton>
#include <QGraphicsOpacityEffect>
#include <QHBoxLayout>
#include <QPropertyAnimation>
#include <QRegularExpression>
#include <QTimer>
#include <QVBoxLayout>
#include <QWidget>

namespace {

bool isDreamHoverChoiceSceneId(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("scene7b_dream_hover_"))
        || sceneId.startsWith(QStringLiteral("dream_3_future_choice"));
}

bool isDreamFutureChoiceSceneId(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("dream_3_future_choice"));
}

bool isDreamSceneId(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("scene7b_dream_"))
        || sceneId.startsWith(QStringLiteral("dream_"));
}

bool isNewDreamSceneId(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("dream_"));
}

bool usesDreamFutureShader(const QString &sceneId)
{
    return sceneId == QStringLiteral("dream_2_suspend");
}

bool isColdOpenMuffledSceneId(const QString &sceneId)
{
    return sceneId.startsWith(QStringLiteral("cold_open_1_"))
        || sceneId.startsWith(QStringLiteral("cold_open_2_"))
        || sceneId.startsWith(QStringLiteral("cold_open_3_"))
        || sceneId.startsWith(QStringLiteral("cold_open_4_"));
}

bool isColdOpenLinglingPortraitSceneId(const QString &sceneId)
{
    return sceneId == QStringLiteral("cold_open_3_lingling");
}

bool looksLikeInternalHeaderId(const QString &text)
{
    static const QRegularExpression internalIdPattern(QStringLiteral("^[A-Za-z0-9_\\-]+$"));
    return internalIdPattern.match(text).hasMatch();
}

QString stripSceneNumberPrefix(const QString &header)
{
    static const QRegularExpression numberedHeaderPattern(
        QString::fromUtf8(u8"^鍦烘櫙[^/]+/\\s*"));
    QString cleaned = header;
    cleaned.remove(numberedHeaderPattern);
    return cleaned.trimmed();
}

int dreamAutoFrameDurationMs(const QString &sceneId)
{
    if (sceneId == QStringLiteral("dream_0_sleep")) {
        return 1900;
    }
    if (sceneId == QStringLiteral("dream_1_fall")) {
        return 1450;
    }
    if (sceneId == QStringLiteral("dream_2_suspend")) {
        return 1850;
    }
    if (sceneId == QStringLiteral("dream_3_future_far")
        || sceneId == QStringLiteral("dream_3_future_together")
        || sceneId == QStringLiteral("dream_3_future_create")) {
        return 1850;
    }
    if (sceneId == QStringLiteral("dream_4_end")) {
        return 2000;
    }
    if (sceneId == QStringLiteral("dream_5_wake")) {
        return 1700;
    }
    if (sceneId == QStringLiteral("scene7b_dream_sleep")) {
        return 2600;
    }
    if (sceneId == QStringLiteral("scene7b_dream_fall")) {
        return 1250;
    }
    if (sceneId == QStringLiteral("scene7b_dream_hover_intro")) {
        return 2100;
    }
    if (sceneId == QStringLiteral("scene7b_dream_close")) {
        return 2200;
    }
    if (sceneId == QStringLiteral("scene7b_dream_fragment_return")) {
        return 1;
    }
    if (sceneId.startsWith(QStringLiteral("scene7b_dream_fragment_"))) {
        return 1900;
    }

    return 0;
}

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
        return {QString::fromUtf8(u8"璇炬鍥?鍌嶆櫄锛?png")};
    case BackgroundStyle::DeskNight:
        return {QString::fromUtf8(u8"璇炬鍥撅紙鏅氫笂锛?png")};
    case BackgroundStyle::Beginning:
        return {QString::fromUtf8(u8"璇炬鍥撅紙beginning锛?png")};
    case BackgroundStyle::BeginningBlur:
        return {QString::fromUtf8(u8"璇炬鍥撅紙beginning_blur锛?png")};
    case BackgroundStyle::Hallway:
        return {QString::fromUtf8(u8"妤奸亾鍥?png")};
    case BackgroundStyle::TreeUnder:
        return {QString::fromUtf8(u8"鏍戜笅.png")};
    case BackgroundStyle::GymBack:
        return {QString::fromUtf8(u8"浣撹偛棣?png")};
    case BackgroundStyle::Office:
        return {QString::fromUtf8(u8"鍔炲叕瀹?png")};
    case BackgroundStyle::Toilet:
        return {QString::fromUtf8(u8"鍘曟墍.png")};
    case BackgroundStyle::Dismissal:
        return {QString::fromUtf8(u8"鏀惧.png")};
    case BackgroundStyle::SchoolGate:
        return {QString::fromUtf8(u8"鏍￠棬鍙?png")};
    case BackgroundStyle::Ebike:
        return {QString::fromUtf8(u8"鐢电摱杞︿笂.png")};
    case BackgroundStyle::Home:
        return {QString::fromUtf8(u8"瀹堕噷.png")};
    case BackgroundStyle::ClassroomDusk:
        return {QString::fromUtf8(u8"璇炬鍥?鍌嶆櫄锛?png")};
    case BackgroundStyle::QuietHallway:
        return {QString::fromUtf8(u8"妤奸亾鍥?png")};
    case BackgroundStyle::ChoiceFocus:
        return {QString::fromUtf8(u8"璇炬鍥撅紙鏅氫笂锛?png")};
    case BackgroundStyle::SoftNarration:
        return {QString::fromUtf8(u8"璇炬鍥撅紙鏅氫笂锛?png")};
    case BackgroundStyle::NightRain:
        return {QString::fromUtf8(u8"鐢电摱杞︿笂.png")};
    case BackgroundStyle::EndingGlow:
    case BackgroundStyle::EndingWhite:
    case BackgroundStyle::EndingBlack:
    case BackgroundStyle::DreamDrift:
        return {};
    case BackgroundStyle::DreamFaraway:
        return {QString::fromUtf8(u8"姊?杩滄柟.png")};
    case BackgroundStyle::DreamCompanion:
        return {QString::fromUtf8(u8"姊?鏈嬪弸.png")};
    case BackgroundStyle::DreamCreation:
        return {QString::fromUtf8(u8"姊?鍒涗綔.png")};
    case BackgroundStyle::Message1:
        return {QString::fromUtf8(u8"娑堟伅1.png")};
    case BackgroundStyle::Message2:
        return {QString::fromUtf8(u8"娑堟伅2.png")};
    case BackgroundStyle::Message3:
        return {QString::fromUtf8(u8"娑堟伅3.png")};
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

QString resolveTitleCoverFilePath()
{
    for (const QString &root : backgroundSearchRoots()) {
        const QString candidate = QDir(root).filePath(QString::fromUtf8(u8"灏侀潰.png"));
        if (QFileInfo::exists(candidate)) {
            return candidate;
        }
    }

    return {};
}

QString resolveDreamShaderFilePath(const QString &fileName)
{
    const QString appDir = QCoreApplication::applicationDirPath();
    const QStringList candidateRoots = {
        QDir(appDir).filePath(QStringLiteral("assets/shaders")),
        QDir(appDir).filePath(QStringLiteral("../assets/shaders")),
        QDir(appDir).filePath(QStringLiteral("../../assets/shaders"))
    };

    for (const QString &root : candidateRoots) {
        if (!QFileInfo::exists(root)) {
            continue;
        }

        QDirIterator it(root,
                        QStringList{fileName},
                        QDir::Files,
                        QDirIterator::Subdirectories);
        if (it.hasNext()) {
            return it.next();
        }
    }

    return {};
}

QString ambientAudioIdForScene(const QString &sceneId)
{
    if (sceneId == QStringLiteral("scene9_bell")) {
        return {};
    }

    if (sceneId == QStringLiteral("dream_0_sleep")) {
        return {};
    }
    if (sceneId == QStringLiteral("dream_2_suspend")
        || sceneId.startsWith(QStringLiteral("dream_3_future_"))
        || sceneId == QStringLiteral("dream_4_end")) {
        return QStringLiteral("amb_dream_air");
    }
    if (sceneId == QStringLiteral("dream_5_wake")) {
        return {};
    }

    if (sceneId == QStringLiteral("cold_open_5_after")) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("cold_open_0_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("cold_open_6_"))) {
        return {};
    }

    if (sceneId.startsWith(QStringLiteral("cold_open_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId == QStringLiteral("scene7b_dream_wake")) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("scene7b_dream_"))) {
        if (sceneId == QStringLiteral("scene7b_dream_sleep")) {
            return QStringLiteral("amb_classroom_evening_and_hallway_loop");
        }
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
        return QStringLiteral("amb_classroom_night_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene5_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("scene6_"))) {
        return QStringLiteral("amb_classroom_evening_and_hallway_loop");
    }

    if (sceneId == QStringLiteral("scene7a_walk_1")
        || sceneId == QStringLiteral("scene7a_walk_2")
        || sceneId == QStringLiteral("scene7a_walk_3")
        || sceneId == QStringLiteral("scene7a_opening")
        || sceneId == QStringLiteral("scene7a_opening_grade_feedback")
        || sceneId == QStringLiteral("scene7a_opening_unexpected_feedback")
        || sceneId == QStringLiteral("scene7a_opening_breakdown_feedback")
        || sceneId == QStringLiteral("scene7a_lingling_slows")
        || sceneId == QStringLiteral("scene7a_tears_begin")
        || sceneId == QStringLiteral("scene7a_tears_fall")
        || sceneId == QStringLiteral("scene7a_hide_place")) {
        return QStringLiteral("amb_playground_night");
    }

    if (sceneId == QStringLiteral("scene7a_topics")
        || sceneId == QStringLiteral("scene7a_hug")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3")) {
        return QStringLiteral("amb_hidden_place_night_loop");
    }

    if (sceneId.startsWith(QStringLiteral("scene7b_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
    }

    if (sceneId.startsWith(QStringLiteral("scene8_"))) {
        return QStringLiteral("amb_office");
    }

    if (sceneId.startsWith(QStringLiteral("scene9_"))) {
        return QStringLiteral("amb_classroom_night_loopc_study");
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
    const bool dreamRoute = gameState.stringValue(QStringLiteral("side_route_mode")) == QString::fromUtf8(u8"姊﹀");
    const bool stayedInClassroomRoute = !gameState.boolValue(QStringLiteral("went_downstairs")) && !dreamRoute;

    if (sceneId == QStringLiteral("dream_2_suspend")
        || sceneId.startsWith(QStringLiteral("dream_3_future_"))
        || sceneId == QStringLiteral("dream_4_end")) {
        return QStringLiteral("bgm_dream_air_walk");
    }

    if (sceneId.startsWith(QStringLiteral("cold_open_"))) {
        return QStringLiteral("bgm_haidi");
    }

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
            if (!state.text.contains(QString::fromUtf8(u8"鐒跺悗浣犲紑濮嬪敱"))) {
                return {};
            }
            if (gameState.stringValue(QStringLiteral("rain_song_emotion")) == QString::fromUtf8(u8"瀹跺涵")) {
                return QStringLiteral("bgm_dont_cry");
            }
            return QStringLiteral("bgm_meiyoulixiang");
        }
        if (sceneId == QStringLiteral("scene10_song_release")
            || sceneId == QStringLiteral("scene10_reply")
            || sceneId == QStringLiteral("scene10_arrive")) {
            if (gameState.stringValue(QStringLiteral("rain_song_emotion")) == QString::fromUtf8(u8"瀹跺涵")) {
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
    if (sceneId == QStringLiteral("dream_2_suspend")) {
        return false;
    }

    if (sceneId.startsWith(QStringLiteral("cold_open_0_"))) {
        return false;
    }

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
    return isDreamSceneId(sceneId)
        || sceneId.startsWith(QStringLiteral("cold_open_0_"))
        || sceneId.startsWith(QStringLiteral("cold_open_6_"))
        || sceneId == QStringLiteral("scene8_breakdown_blackout")
        || sceneId == QStringLiteral("scene9_mainline_end")
        || sceneId == QStringLiteral("scene10_song_release")
        || sceneId == QStringLiteral("scene7a_comfort")
        || sceneId == QStringLiteral("scene7a_comfort_2")
        || sceneId == QStringLiteral("scene7a_comfort_3");
}

bool shouldKeepAmbientUnderBgm(const QString &sceneId)
{
    if (sceneId.startsWith(QStringLiteral("cold_open_"))) {
        return true;
    }

    return sceneId.startsWith(QStringLiteral("scene3_"))
        || sceneId.startsWith(QStringLiteral("scene4_"))
        || sceneId.startsWith(QStringLiteral("scene5_"))
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
    if (sceneId == QStringLiteral("dream_1_fall")) {
        return QStringLiteral("emo_dream_fall_low");
    }
    if (sceneId == QStringLiteral("cold_open_6_black_retry")) {
        return QStringLiteral("emo_dream_fall_low");
    }
    if (sceneId == QStringLiteral("cold_open_1_score")) {
        return QStringLiteral("emo_muffled_ring");
    }
    if (sceneId == QStringLiteral("scene4_score_fall")) {
        return QStringLiteral("emo_score_low_rumble");
    }
    if (sceneId == QStringLiteral("scene7b_dream_sleep")) {
        return QStringLiteral("emo_muffled_ring");
    }
    if (sceneId == QStringLiteral("scene7b_dream_fall")) {
        return QStringLiteral("emo_score_low_rumble");
    }
    if (sceneId == QStringLiteral("scene8_teacher_first_sting")) {
        return QStringLiteral("emo_muffled_ring");
    }

    return {};
}

QString sceneEntrySfxAudioId(const QString &sceneId)
{
    if (sceneId == QStringLiteral("cold_open_0_number")) {
        return QStringLiteral("sfx_bell");
    }
    if (sceneId == QStringLiteral("scene8_enter_prompt")) {
        return QStringLiteral("sfx_sink_water");
    }
    if (sceneId == QStringLiteral("scene3_class_bell")
        || sceneId == QStringLiteral("scene4_break_bell")
        || sceneId == QStringLiteral("scene5_period_bell")
        || sceneId == QStringLiteral("scene6_invitation_bell")
        || sceneId == QStringLiteral("scene7b_sleep_fail_bell")
        || sceneId == QStringLiteral("scene10_departure_bell")) {
        return QStringLiteral("sfx_bell");
    }
    if (sceneId == QStringLiteral("scene9_bell")) {
        return QStringLiteral("sfx_bell");
    }
    if (sceneId == QStringLiteral("scene7b_dream_wake")) {
        return QStringLiteral("sfx_bell");
    }

    return {};
}

} // namespace

GameWindow::GameWindow(QWidget *parent)
    : QMainWindow(parent),
      m_backgroundWidget(nullptr),
      m_dreamFadeOverlay(nullptr),
      m_dreamFadeEffect(nullptr),
      m_dreamFadeAnimation(nullptr),
      m_shaderWidget(nullptr),
      m_characterPortraitLabel(nullptr),
      m_dialoguePanel(nullptr),
      m_floatingChoiceLayer(nullptr),
      m_centerTextLabel(nullptr),
      m_headerLabel(nullptr),
      m_titleScreenOverlay(nullptr),
      m_titleBackgroundLabel(nullptr),
      m_titleStartButton(nullptr),
      m_titleScreenEffect(nullptr),
      m_titleScreenFadeAnimation(nullptr),
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
      m_titleScreenActive(false),
      m_titleScreenTransitioning(false),
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
    showTitleScreen();
}

void GameWindow::setSpeaker(const QString &speaker)
{
    m_dialoguePanel->setSpeaker(displaySpeakerName(speaker));
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
        if (m_shaderWidget) {
            m_shaderWidget->setSourcePixmap(QPixmap());
        }
        return;
    }

    m_backgroundWidget->setBackgroundPixmap(pixmap);
    if (m_shaderWidget) {
        m_shaderWidget->setSourcePixmap(pixmap);
    }
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
    hideFloatingChoiceLayer();
    clearCenterText();
    setDialogueVisible(true);
    clearSpeaker();
    updateDialoguePanelBounds(false, 0);
    setPagedText(text, showContinue);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
}

void GameWindow::showDialogueMode(const QString &speaker, const QString &text, bool showContinue)
{
    hideFloatingChoiceLayer();
    clearCenterText();
    setDialogueVisible(true);
    setSpeaker(speaker);
    updateDialoguePanelBounds(false, 0);
    setPagedText(text, showContinue);
    setInteractionMode(InteractionMode::None);
    setInteractionItems({});
    m_dialoguePanel->setInteractionVisible(false);
}

void GameWindow::showPerformanceMode(const QString &text)
{
    hideFloatingChoiceLayer();
    if (isMessageNotificationScene(m_activeSceneId)) {
        showMessageNotificationMode(text);
        return;
    }

    resetMessageNotificationOverlay();
    setDialogueVisible(false);
    clearPagedText();
    updateDialoguePanelBounds(false, 0);
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
            startCenterTextTypewriter(displayCenterText(m_textFrames.first()), 58);
        } else {
            stopCenterTextTypewriter();
            setCenterText(displayCenterText(m_textFrames.first()));
        }
    } else if (shouldTypewriteCenterText(m_activeSceneId)) {
        m_centerTextFrameMode = false;
        m_currentTextFrameIndex = -1;
        startCenterTextTypewriter(displayCenterText(text), 58);
    } else {
        m_centerTextFrameMode = false;
        m_currentTextFrameIndex = -1;
        stopCenterTextTypewriter();
        setCenterText(displayCenterText(text));
    }
}

void GameWindow::showInteractionMode(const QString &speaker,
                                     const QString &text,
                                     InteractionMode mode,
                                     const InteractionItems &items,
                                     bool showContinue)
{
    const bool useFloatingChoiceLayer = shouldUseFloatingChoiceLayer(mode, items);
    hideFloatingChoiceLayer();
    clearCenterText();
    setDialogueVisible(true);
    clearPagedText();

    const QString displaySpeaker = displaySpeakerName(speaker);
    if (displaySpeaker.trimmed().isEmpty()) {
        clearSpeaker();
    } else {
        setSpeaker(displaySpeaker);
    }

    const QStringList customFrames = buildCustomTextFrames(text);
    if (!customFrames.isEmpty()) {
        m_sourceText = text;
        m_textFrames = customFrames;
        m_currentTextFrameIndex = 0;
        setText(m_textFrames.first());
        setInteractionMode(mode);
        setInteractionItems(items);
        updateDialoguePanelBounds(useFloatingChoiceLayer ? false : !items.isEmpty(),
                                  m_dialoguePanel->visibleInteractionCount());
        m_dialoguePanel->setInteractionVisible(false);
        setContinueVisible(true);
        return;
    }

    setText(text);
    setInteractionMode(mode);
    setInteractionItems(items);
    updateDialoguePanelBounds(useFloatingChoiceLayer ? false : !items.isEmpty(),
                              m_dialoguePanel->visibleInteractionCount());
    if (useFloatingChoiceLayer) {
        m_dialoguePanel->setInteractionVisible(false);
        showFloatingChoiceLayer(items);
        setContinueVisible(false);
        return;
    }

    m_dialoguePanel->setInteractionVisible(!items.isEmpty());
    setContinueVisible(showContinue);

    if (!items.isEmpty()) {
        m_dialoguePanel->focusFirstInteraction();
    }
}

void GameWindow::keyPressEvent(QKeyEvent *event)
{
    if (m_titleScreenActive && !m_titleScreenTransitioning) {
        if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter) {
            startGameFromTitle();
            event->accept();
            return;
        }
    }

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

    updateDreamFadeGeometry();
    updateMessageNotificationGeometry();
    updateTitleScreenGeometry();
    updateDialoguePanelBounds(m_dialoguePanel && m_dialoguePanel->hasVisibleInteractions(),
                              m_dialoguePanel ? m_dialoguePanel->visibleInteractionCount() : 0);
    updateFloatingChoiceLayerGeometry();
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
            if (shouldUseFloatingChoiceLayer(m_lastNarrativeViewState->interactionMode,
                                             m_lastNarrativeViewState->interactionItems)) {
                m_dialoguePanel->setInteractionVisible(false);
                showFloatingChoiceLayer(m_lastNarrativeViewState->interactionItems);
            } else {
                m_dialoguePanel->setInteractionVisible(true);
                m_dialoguePanel->focusFirstInteraction();
            }
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
    const bool hasVisibleBottomInteractions = m_dialoguePanel && m_dialoguePanel->hasVisibleInteractions();
    const bool hasVisibleFloatingChoices = m_floatingChoiceLayer
        && m_floatingChoiceLayer->isVisible()
        && m_floatingChoiceLayer->visibleChoiceCount() > 0;

    if (!hasVisibleBottomInteractions && !hasVisibleFloatingChoices) {
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
    setHeaderText(displayHeaderText(state.header));
    if (!isDreamSceneId(state.sceneId)) {
        clearDreamShaderOverlay();
    }
    setBackgroundStyle(state.backgroundStyle);
    setShaderEffect(state.shaderEffect);
    applyDreamShaderBinding(state);
    applyDreamPresentation(state, sceneChanged);

    if (state.displayMode == NarrativeDisplayMode::Interaction && !state.interactionItems.isEmpty()) {
        showInteractionMode(state.showSpeaker ? state.speaker : QString(),
                            state.text,
                            state.interactionMode,
                            state.interactionItems,
                            state.showContinue);
        if (state.autoAdvance) {
            scheduleAutoAdvance(state);
        }
        updateCharacterPortrait(state);
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
    updateCharacterPortrait(state);
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
    setWindowTitle(QString::fromUtf8(u8"第一天"));
    menuBar()->hide();

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
    m_headerLabel->setStyleSheet(
        "QLabel {"
        "    color: rgba(232, 240, 255, 176);"
        "    background: transparent;"
        "    border: none;"
        "    padding: 2px 4px;"
        "    font-size: 16px;"
        "    letter-spacing: 0px;"
        "    font-weight: 500;"
        "}"
    );
    m_headerLabel->hide();
    layout->addWidget(m_headerLabel, 0, Qt::AlignLeft | Qt::AlignTop);

    layout->addStretch();

    m_dreamFadeOverlay = new QWidget(m_backgroundWidget);
    m_dreamFadeOverlay->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_dreamFadeOverlay->setStyleSheet(QStringLiteral("background: rgb(0, 0, 0);"));
    m_dreamFadeOverlay->hide();
    m_dreamFadeEffect = new QGraphicsOpacityEffect(m_dreamFadeOverlay);
    m_dreamFadeEffect->setOpacity(0.0);
    m_dreamFadeOverlay->setGraphicsEffect(m_dreamFadeEffect);
    m_dreamFadeAnimation = new QPropertyAnimation(m_dreamFadeEffect, "opacity", m_dreamFadeOverlay);
    connect(m_dreamFadeAnimation, &QPropertyAnimation::finished, this, [this]() {
        if (m_dreamFadeEffect && m_dreamFadeEffect->opacity() <= 0.01 && m_dreamFadeOverlay) {
            m_dreamFadeOverlay->hide();
        }
    });
    updateDreamFadeGeometry();

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
    updateCenterTextAppearance();
    m_centerTextLabel->hide();
    layout->addWidget(m_centerTextLabel, 0, Qt::AlignHCenter | Qt::AlignVCenter);

    layout->addStretch();

    m_characterPortraitLabel = new QLabel(m_backgroundWidget);
    m_characterPortraitLabel->setAttribute(Qt::WA_TransparentForMouseEvents);
    m_characterPortraitLabel->setStyleSheet(QStringLiteral("background: transparent;"));
    m_characterPortraitLabel->setScaledContents(true);
    m_characterPortraitLabel->hide();

    m_dialoguePanel = new DialoguePanel(m_backgroundWidget);
    updateDialoguePanelBounds(false, 0);
    m_dialoguePanel->show();
    m_dialoguePanel->hide();

    m_floatingChoiceLayer = new FloatingChoiceLayer(m_backgroundWidget);
    m_floatingChoiceLayer->setGeometry(m_backgroundWidget->rect());
    m_floatingChoiceLayer->hide();

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

    m_messageNotificationContinueButton = new QPushButton(QString::fromUtf8(u8"缁х画"), m_messageNotificationCard);
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
    connect(m_shaderWidget, &ShaderToyWidget::shaderCompileFailed, this, [this](const QString &) {
        clearDreamShaderOverlay();
    });

    m_dreamFadeOverlay->raise();
    m_headerLabel->raise();
    m_centerTextLabel->raise();
    m_dialoguePanel->raise();
    m_floatingChoiceLayer->raise();
    m_messageNotificationOverlay->raise();
    buildTitleScreen();

    connect(m_dialoguePanel, &DialoguePanel::continueRequested, this, &GameWindow::advanceNarrative);
    connect(m_dialoguePanel, &DialoguePanel::continueButtonClicked, this, &GameWindow::handleContinueButtonClicked);
    connect(m_dialoguePanel, &DialoguePanel::interactionTriggered, this, &GameWindow::handleInteractionTriggered);
    connect(m_dialoguePanel, &DialoguePanel::interactionButtonClicked, this, &GameWindow::handleInteractionButtonClicked);
    connect(m_floatingChoiceLayer, &FloatingChoiceLayer::choiceTriggered, this, &GameWindow::handleInteractionTriggered);
    connect(m_floatingChoiceLayer, &FloatingChoiceLayer::choiceButtonClicked, this, &GameWindow::handleInteractionButtonClicked);
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

void GameWindow::buildTitleScreen()
{
    if (!m_backgroundWidget || m_titleScreenOverlay) {
        return;
    }

    m_titleCoverPixmap = resolveTitleCoverPixmap();

    m_titleScreenOverlay = new QWidget(m_backgroundWidget);
    m_titleScreenOverlay->setObjectName(QStringLiteral("titleScreenOverlay"));
    m_titleScreenOverlay->setStyleSheet(QStringLiteral(
        "QWidget#titleScreenOverlay {"
        "    background: transparent;"
        "}"));

    m_titleBackgroundLabel = new QLabel(m_titleScreenOverlay);
    m_titleBackgroundLabel->setAlignment(Qt::AlignCenter);
    m_titleBackgroundLabel->setStyleSheet(QStringLiteral("background: transparent;"));

    m_titleStartButton = new QPushButton(QString(), m_titleScreenOverlay);
    m_titleStartButton->setObjectName(QStringLiteral("titleStartButton"));
    m_titleStartButton->setCursor(Qt::PointingHandCursor);
    m_titleStartButton->setFocusPolicy(Qt::StrongFocus);
    m_titleStartButton->setFlat(true);
    m_titleStartButton->setToolTip(QString::fromUtf8(u8"开始游戏"));
    m_titleStartButton->setStyleSheet(QStringLiteral(
        "QPushButton#titleStartButton {"
        "    color: transparent;"
        "    background: rgba(255, 255, 255, 0);"
        "    border: 1px solid rgba(243, 234, 220, 0);"
        "    border-radius: 16px;"
        "    padding: 0px;"
        "}"
        "QPushButton#titleStartButton:hover {"
        "    background: rgba(248, 240, 226, 26);"
        "    border: 1px solid rgba(244, 236, 222, 92);"
        "}"
        "QPushButton#titleStartButton:pressed {"
        "    background: rgba(236, 226, 211, 42);"
        "    border: 1px solid rgba(244, 236, 222, 118);"
        "}"
        "QPushButton#titleStartButton:focus {"
        "    border: 1px solid rgba(244, 236, 222, 82);"
        "}"));
    connect(m_titleStartButton, &QPushButton::clicked, this, &GameWindow::startGameFromTitle);

    m_titleScreenEffect = new QGraphicsOpacityEffect(m_titleScreenOverlay);
    m_titleScreenEffect->setOpacity(1.0);
    m_titleScreenOverlay->setGraphicsEffect(m_titleScreenEffect);

    m_titleScreenFadeAnimation = new QPropertyAnimation(m_titleScreenEffect, "opacity", m_titleScreenOverlay);
    m_titleScreenFadeAnimation->setDuration(420);
    m_titleScreenFadeAnimation->setEasingCurve(QEasingCurve::InOutQuad);
    connect(m_titleScreenFadeAnimation, &QPropertyAnimation::finished, this, &GameWindow::finishTitleScreenTransition);

    updateTitleScreenGeometry();
    m_titleScreenOverlay->hide();
}

void GameWindow::showTitleScreen()
{
    if (!m_titleScreenOverlay) {
        return;
    }

    m_titleScreenActive = true;
    m_titleScreenTransitioning = false;
    m_activeSceneId.clear();
    clearCenterText();
    clearSpeaker();
    clearPagedText();
    setContinueVisible(false);
    m_dialoguePanel->hide();
    m_headerLabel->hide();
    m_characterPortraitLabel->hide();
    resetMessageNotificationOverlay();
    m_autoAdvanceTimer->stop();
    m_bgmStartDelayTimer->stop();
    m_ambStopAfterBgmTimer->stop();
    if (m_audioManager) {
        m_audioManager->stopBgm(300);
        m_audioManager->stopAmb(300);
    }

    if (m_titleScreenFadeAnimation) {
        m_titleScreenFadeAnimation->stop();
    }
    if (m_titleScreenEffect) {
        m_titleScreenEffect->setOpacity(1.0);
    }

    updateTitleScreenGeometry();
    m_titleScreenOverlay->show();
    m_titleScreenOverlay->raise();
    if (m_titleStartButton) {
        m_titleStartButton->setEnabled(true);
        m_titleStartButton->setFocus();
    }
}

void GameWindow::updateTitleScreenGeometry()
{
    if (!m_backgroundWidget || !m_titleScreenOverlay) {
        return;
    }

    const QRect rect = m_backgroundWidget->rect();
    m_titleScreenOverlay->setGeometry(rect);

    if (m_titleBackgroundLabel) {
        m_titleBackgroundLabel->setGeometry(rect);

        if (!m_titleCoverPixmap.isNull()) {
            const QPixmap scaled = m_titleCoverPixmap.scaled(rect.size(),
                                                             Qt::KeepAspectRatioByExpanding,
                                                             Qt::SmoothTransformation);
            m_titleBackgroundLabel->setPixmap(scaled);
        } else {
            m_titleBackgroundLabel->clear();
        }
    }

    if (m_titleStartButton) {
        const int buttonWidth = qMax(228, qRound(rect.width() * 0.18));
        const int buttonHeight = qMax(56, qRound(rect.height() * 0.074));
        const int x = (rect.width() - buttonWidth) / 2;
        const int y = qRound(rect.height() * 0.755);
        m_titleStartButton->setGeometry(x, y, buttonWidth, buttonHeight);
        m_titleStartButton->raise();
    }
}

void GameWindow::startGameFromTitle()
{
    if (!m_titleScreenActive || m_titleScreenTransitioning || !m_titleScreenOverlay) {
        return;
    }

    m_titleScreenTransitioning = true;
    playUiClickSound();
    if (m_titleStartButton) {
        m_titleStartButton->setEnabled(false);
    }

    if (m_titleScreenFadeAnimation && m_titleScreenEffect) {
        m_titleScreenFadeAnimation->stop();
        m_titleScreenFadeAnimation->setStartValue(m_titleScreenEffect->opacity());
        m_titleScreenFadeAnimation->setEndValue(0.0);
        m_titleScreenFadeAnimation->start();
        return;
    }

    finishTitleScreenTransition();
}

void GameWindow::finishTitleScreenTransition()
{
    if (!m_titleScreenActive) {
        return;
    }

    m_titleScreenTransitioning = false;
    m_titleScreenActive = false;
    if (m_titleScreenOverlay) {
        m_titleScreenOverlay->hide();
    }
    if (m_titleScreenEffect) {
        m_titleScreenEffect->setOpacity(1.0);
    }
    if (m_dialoguePanel) {
        m_dialoguePanel->show();
    }
    m_narrativeEngine->start();
}

void GameWindow::syncAmbientAudio(const NarrativeViewState &state)
{
    if (!m_audioManager) {
        return;
    }

    QString audioId = ambientAudioIdForScene(state.sceneId);
    if ((state.sceneId.startsWith(QStringLiteral("scene1_"))
         || state.sceneId.startsWith(QStringLiteral("scene2_")))
        && state.speaker == QString::fromUtf8(u8"宸濆摜")
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
        const int fadeOutMs = (state.sceneId == QStringLiteral("scene7b_dream_fall")
            || state.sceneId == QStringLiteral("dream_1_fall")) ? 1800 : 800;
        m_audioManager->stopAmb(fadeOutMs);
        return;
    }

    const bool enteringHiddenPlace = state.sceneId == QStringLiteral("scene7a_topics")
        && audioId == QStringLiteral("amb_hidden_place_night_loop");
    const bool enteringDreamAir = state.sceneId == QStringLiteral("dream_2_suspend")
        && audioId == QStringLiteral("amb_dream_air");
    m_audioManager->playAmb(audioId,
                            enteringHiddenPlace ? 120 : 800,
                            enteringHiddenPlace ? 220 : (enteringDreamAir ? 1500 : 800));
    if (state.sceneId == QStringLiteral("scene7b_dream_sleep")) {
        m_audioManager->setAmbVolumeScale(0.25, 2200);
    } else if (state.sceneId == QStringLiteral("dream_0_sleep")) {
        m_audioManager->setAmbVolumeScale(0.20, 2000);
    } else if (state.sceneId == QStringLiteral("dream_2_suspend")) {
        m_audioManager->setAmbVolumeScale(1.0, 1500);
    } else if (state.sceneId.startsWith(QStringLiteral("dream_3_future_"))) {
        m_audioManager->restoreAmb(700);
    } else if (state.sceneId == QStringLiteral("dream_4_end")) {
        m_audioManager->setAmbVolumeScale(0.72, 900);
    } else if (state.sceneId == QStringLiteral("dream_5_wake")) {
        m_audioManager->restoreAmb(1400);
    } else if (state.sceneId.startsWith(QStringLiteral("cold_open_0_"))) {
        m_audioManager->setAmbVolumeScale(0.22, 1200);
    } else if (isColdOpenMuffledSceneId(state.sceneId)
               || state.sceneId == QStringLiteral("cold_open_3_lingling")) {
        m_audioManager->setAmbVolumeScale(0.32, 1000);
    } else if (state.sceneId.startsWith(QStringLiteral("cold_open_"))) {
        m_audioManager->restoreAmb(700);
    } else if (state.sceneId == QStringLiteral("scene7b_dream_wake")) {
        m_audioManager->restoreAmb(1800);
    } else {
        m_audioManager->restoreAmb(700);
    }
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
        const int fadeOutMs = state.sceneId == QStringLiteral("dream_5_wake")
            ? 3000
            : (sceneChanged ? 0 : 800);
        m_audioManager->stopBgm(fadeOutMs);
        return;
    }

    if (currentBgmId == audioId && m_pendingBgmAudioId.isEmpty()) {
        const int ambLeadOutDelayMs = ambientLeadOutDelayMsForScene(state.sceneId);
        if (sceneChanged && ambLeadOutDelayMs > 0 && !m_audioManager->currentAmbId().isEmpty()) {
            m_ambStopAfterBgmTimer->start(ambLeadOutDelayMs);
        }
        if (state.sceneId.startsWith(QStringLiteral("scene4_"))) {
            m_audioManager->duckBgm(0.18, 500);
        } else if (state.sceneId == QStringLiteral("dream_4_end")) {
            m_audioManager->duckBgm(0.70, 900);
        } else {
            m_audioManager->restoreBgm(state.sceneId == QStringLiteral("dream_2_suspend") ? 2500 : 500);
        }
        return;
    }

    if (!sceneChanged && m_pendingBgmAudioId == audioId) {
        return;
    }

    if (!shouldDelayBgmStart(state.sceneId)) {
        m_pendingBgmAudioId.clear();
        m_bgmStartDelayTimer->stop();
        const int fadeInMs = state.sceneId == QStringLiteral("dream_2_suspend") ? 2500 : 800;
        m_audioManager->playBgm(audioId, sceneChangeFadeOutMs, fadeInMs);
        const int ambLeadOutDelayMs = ambientLeadOutDelayMsForScene(state.sceneId);
        if (ambLeadOutDelayMs > 0 && !m_audioManager->currentAmbId().isEmpty()) {
            m_ambStopAfterBgmTimer->start(ambLeadOutDelayMs);
        }
        if (state.sceneId.startsWith(QStringLiteral("scene4_"))) {
            m_audioManager->duckBgm(0.18, 500);
        } else if (state.sceneId == QStringLiteral("dream_4_end")) {
            m_audioManager->duckBgm(0.70, 900);
        } else {
            m_audioManager->restoreBgm(state.sceneId == QStringLiteral("dream_2_suspend") ? 2500 : 500);
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

    if (state.sceneId == QStringLiteral("scene1_intro")) {
        m_audioManager->stopEmo(QStringLiteral("emo_dream_fall_low"));
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
    updateDialoguePanelBounds(false, 0);
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

void GameWindow::updateDreamFadeGeometry()
{
    if (!m_backgroundWidget || !m_dreamFadeOverlay) {
        return;
    }

    m_dreamFadeOverlay->setGeometry(m_backgroundWidget->rect());
}

void GameWindow::clearDreamFade()
{
    if (!m_dreamFadeAnimation || !m_dreamFadeEffect || !m_dreamFadeOverlay) {
        return;
    }

    m_dreamFadeAnimation->stop();
    m_dreamFadeEffect->setOpacity(0.0);
    m_dreamFadeOverlay->hide();
}

void GameWindow::startDreamFade(qreal startOpacity, qreal endOpacity, int durationMs)
{
    if (!m_dreamFadeAnimation || !m_dreamFadeEffect || !m_dreamFadeOverlay) {
        return;
    }

    updateDreamFadeGeometry();
    m_dreamFadeAnimation->stop();
    m_dreamFadeOverlay->show();
    m_dreamFadeOverlay->raise();
    m_dreamFadeEffect->setOpacity(startOpacity);
    m_dreamFadeAnimation->setDuration(qMax(1, durationMs));
    m_dreamFadeAnimation->setStartValue(startOpacity);
    m_dreamFadeAnimation->setEndValue(endOpacity);
    m_dreamFadeAnimation->start();
    if (m_headerLabel) {
        m_headerLabel->raise();
    }
    if (m_centerTextLabel) {
        m_centerTextLabel->raise();
    }
    if (m_dialoguePanel) {
        m_dialoguePanel->raise();
    }
    if (m_messageNotificationOverlay && m_messageNotificationOverlay->isVisible()) {
        m_messageNotificationOverlay->raise();
    }
}

void GameWindow::applyDreamPresentation(const NarrativeViewState &state, bool sceneChanged)
{
    if (!sceneChanged) {
        return;
    }

    if (!isDreamSceneId(state.sceneId)) {
        clearDreamFade();
        return;
    }

    if (!isNewDreamSceneId(state.sceneId)) {
        clearDreamFade();
        return;
    }

    if (state.sceneId == QStringLiteral("dream_0_sleep")) {
        startDreamFade(0.0, 1.0, 3400);
        return;
    }

    if (state.sceneId == QStringLiteral("dream_1_fall")) {
        clearDreamFade();
        return;
    }

    if (state.sceneId == QStringLiteral("dream_2_suspend")) {
        startDreamFade(1.0, 0.0, 1800);
        return;
    }

    if (state.sceneId == QStringLiteral("dream_3_future_far")
        || state.sceneId == QStringLiteral("dream_3_future_together")
        || state.sceneId == QStringLiteral("dream_3_future_create")) {
        startDreamFade(1.0, 0.0, 700);
        return;
    }

    if (state.sceneId == QStringLiteral("dream_4_end")) {
        startDreamFade(0.0, 1.0, 3200);
        return;
    }

    if (state.sceneId == QStringLiteral("dream_5_wake")) {
        startDreamFade(1.0, 0.0, 1400);
        return;
    }

    clearDreamFade();
}

void GameWindow::applyDreamShaderBinding(const NarrativeViewState &state)
{
    if (!m_shaderWidget) {
        return;
    }

    QString shaderFileName;
    bool useNoiseTexture = false;
    if (state.sceneId == QStringLiteral("dream_1_fall")) {
        shaderFileName = QStringLiteral("dream_fall_xskgrw.frag");
    } else if (usesDreamFutureShader(state.sceneId)) {
        shaderFileName = QStringLiteral("dream_future_mtcgdf.frag");
        useNoiseTexture = true;
    } else {
        clearDreamShaderOverlay();
        return;
    }

    const QString shaderFilePath = resolveDreamShaderFilePath(shaderFileName);
    if (shaderFilePath.isEmpty()) {
        qWarning() << "Dream shader file not found:" << shaderFileName;
        clearDreamShaderOverlay();
        return;
    }

    setDreamShaderOverlayFile(shaderFilePath);
    setDreamShaderOverlayUseNoiseTexture(useNoiseTexture);
    setDreamShaderOverlayEnabled(true);
}

void GameWindow::setDreamShaderOverlayEnabled(bool enabled)
{
    if (!m_shaderWidget) {
        return;
    }

    m_shaderWidget->setDreamShaderEnabled(enabled);
    if (!enabled) {
        if (m_shaderWidget->shaderEffect() == ShaderEffect::None) {
            m_shaderWidget->hide();
        }
        return;
    }

    if (!m_shaderWidget->externalFragmentShaderFile().isEmpty()) {
        m_shaderWidget->restartAnimation();
        m_shaderWidget->show();
        m_shaderWidget->raise();
        if (m_characterPortraitLabel && m_characterPortraitLabel->isVisible()) {
            m_characterPortraitLabel->raise();
        }
        if (m_headerLabel) {
            m_headerLabel->raise();
        }
        if (m_centerTextLabel) {
            m_centerTextLabel->raise();
        }
        if (m_dialoguePanel) {
            m_dialoguePanel->raise();
        }
    }
}

void GameWindow::setDreamShaderOverlayFile(const QString &shaderFilePath)
{
    if (!m_shaderWidget) {
        return;
    }

    m_shaderWidget->setExternalFragmentShaderFile(shaderFilePath);
}

void GameWindow::setDreamShaderOverlayUseNoiseTexture(bool useNoiseTexture)
{
    if (!m_shaderWidget) {
        return;
    }

    m_shaderWidget->setExternalChannel0UsesNoiseTexture(useNoiseTexture);
}

void GameWindow::clearDreamShaderOverlay()
{
    if (!m_shaderWidget) {
        return;
    }

    m_shaderWidget->setDreamShaderEnabled(false);
    m_shaderWidget->setExternalFragmentShaderFile(QString());
    m_shaderWidget->setExternalChannel0UsesNoiseTexture(false);
    if (m_shaderWidget->shaderEffect() == ShaderEffect::None) {
        m_shaderWidget->hide();
    }
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
    const QString cleaned = text.trimmed();
    m_headerLabel->setText(cleaned);
    m_headerLabel->setVisible(!cleaned.isEmpty());
}

void GameWindow::setCenterText(const QString &text)
{
    updateCenterTextAppearance();
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

void GameWindow::updateCenterTextAppearance()
{
    if (!m_centerTextLabel) {
        return;
    }

    const QString textColor = isColdOpenCenterTextScene(m_activeSceneId)
        ? QStringLiteral("#FF8E8E")
        : QStringLiteral("#EEF4FF");
    m_centerTextLabel->setStyleSheet(
        QStringLiteral(
            "QLabel#centerTextLabel {"
            "    color: %1;"
            "    font-size: 24px;"
            "    line-height: 155%;"
            "    font-weight: 500;"
            "    background: transparent;"
            "}").arg(textColor));
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
            startCenterTextTypewriter(displayCenterText(nextFrame), 58);
        } else {
            stopCenterTextTypewriter();
            setCenterText(displayCenterText(nextFrame));
        }
    } else {
        setText(nextFrame);
        setContinueVisible(true);
    }
    if (m_lastNarrativeViewState && m_activeSceneId == QStringLiteral("scene7a_hug")) {
        updateCharacterPortrait(*m_lastNarrativeViewState);
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
        setCenterText(displayCenterText(m_textFrames.at(m_currentTextFrameIndex)));
    } else {
        setText(m_textFrames.at(m_currentTextFrameIndex));
    }
}

void GameWindow::updateDialoguePanelBounds(bool hasInteractions, int interactionCount)
{
    if (!m_dialoguePanel || !m_backgroundWidget) {
        return;
    }

    const QRect rect = m_backgroundWidget->rect();
    const bool isDreamHoverChoice = isDreamHoverChoiceSceneId(m_activeSceneId) && hasInteractions;
    if (isDreamHoverChoice) {
        const int panelWidth = qMax(980, qRound(rect.width() * 0.58));
        const int panelHeight = qMax(300, qRound(rect.height() * 0.28));
        const int x = (rect.width() - panelWidth) / 2;
        const int y = qRound(rect.height() * 0.60) - panelHeight / 2;
        m_dialoguePanel->setGeometry(x, y, panelWidth, panelHeight);
        m_dialoguePanel->raise();
        return;
    }

    const bool isScene1Interaction = m_activeSceneId.startsWith(QStringLiteral("scene1_")) && hasInteractions;
    if (isScene1Interaction) {
        const int panelWidth = qMax(1140, qRound(rect.width() * 0.86));
        const int panelHeight = qMax(196, qRound(rect.height() * 0.255));
        const int x = (rect.width() - panelWidth) / 2;
        const int y = rect.height() - panelHeight - 18;
        m_dialoguePanel->setGeometry(x, y, panelWidth, panelHeight);
        m_dialoguePanel->raise();
        return;
    }

    const int panelWidth = hasInteractions
        ? qMax(980, qRound(rect.width() * 0.79))
        : qMax(940, qRound(rect.width() * 0.75));
    int panelHeight = qMax(168, qRound(rect.height() * 0.225));
    if (hasInteractions) {
        if (interactionCount <= 2) {
            panelHeight = qMax(panelHeight, qRound(rect.height() * 0.265));
        } else {
            panelHeight = qMax(panelHeight, qRound(rect.height() * 0.285));
        }
    }
    const int x = (rect.width() - panelWidth) / 2;
    const int y = rect.height() - panelHeight - 22;

    m_dialoguePanel->setGeometry(x, y, panelWidth, panelHeight);
    m_dialoguePanel->raise();
}

void GameWindow::updateFloatingChoiceLayerGeometry()
{
    if (!m_floatingChoiceLayer || !m_backgroundWidget) {
        return;
    }

    m_floatingChoiceLayer->setGeometry(m_backgroundWidget->rect());

    int leftAvoidance = qRound(m_backgroundWidget->width() * 0.18);
    if (m_characterPortraitLabel && m_characterPortraitLabel->isVisible()) {
        leftAvoidance = qMax(leftAvoidance, m_characterPortraitLabel->geometry().right() + 26);
    }

    m_floatingChoiceLayer->setLeftAvoidance(leftAvoidance);
    m_floatingChoiceLayer->raise();
}

bool GameWindow::shouldUseFloatingChoiceLayer(InteractionMode mode, const InteractionItems &items) const
{
    if (mode == InteractionMode::Object || mode == InteractionMode::None) {
        return false;
    }

    if (mode != InteractionMode::Choice
        && mode != InteractionMode::Hotspot
        && mode != InteractionMode::Topic
        && mode != InteractionMode::Action) {
        return false;
    }

    int visibleCount = 0;
    for (const InteractionItem &item : items) {
        if (!item.hidden) {
            ++visibleCount;
        }
    }

    if (visibleCount > 2) {
        qWarning() << "Floating choice layer fallback to bottom layout for scene"
                   << m_activeSceneId
                   << "mode"
                   << static_cast<int>(mode)
                   << "visible interactions"
                   << visibleCount;
    }

    return visibleCount == 1 || visibleCount == 2;
}

void GameWindow::showFloatingChoiceLayer(const InteractionItems &items)
{
    if (!m_floatingChoiceLayer) {
        return;
    }

    m_floatingChoiceLayer->setChoices(items);
    if (!m_floatingChoiceLayer->hasChoices()) {
        m_floatingChoiceLayer->hide();
        return;
    }

    updateFloatingChoiceLayerGeometry();
    m_floatingChoiceLayer->show();
    m_floatingChoiceLayer->raise();
}

void GameWindow::hideFloatingChoiceLayer()
{
    if (!m_floatingChoiceLayer) {
        return;
    }

    m_floatingChoiceLayer->clearChoices();
    m_floatingChoiceLayer->hide();
}

QString GameWindow::displayHeaderText(const QString &text) const
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty() || looksLikeInternalHeaderId(trimmed)) {
        return {};
    }

    return stripSceneNumberPrefix(trimmed);
}

QString GameWindow::displaySpeakerName(const QString &speaker) const
{
    const QString trimmed = speaker.trimmed();
    if (trimmed.isEmpty() || trimmed == QString::fromUtf8(u8"鏃佺櫧")) {
        return {};
    }

    return trimmed;
}

QString GameWindow::displayCenterText(const QString &text) const
{
    const QString trimmed = text.trimmed();
    if (trimmed.isEmpty()) {
        return {};
    }

    return trimmed + QString::fromUtf8(u8"\n▼");
}

bool GameWindow::isColdOpenCenterTextScene(const QString &sceneId) const
{
    return sceneId.startsWith(QStringLiteral("cold_open_"));
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
    const bool inColdOpenLingling = isColdOpenLinglingPortraitSceneId(state.sceneId);
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
            && state.speaker == QString::fromUtf8(u8"宸濆摜"))
        || (inOfficeScene && !inOfficeDoorway && !inOfficeReturnMonologue && !inOfficeBlackout);
    const bool shouldShowLingling = inLinglingScene6
        || inColdOpenLingling
        || inLinglingOpening
        || inLinglingTopics
        || inLinglingHug
        || inLinglingComfort;
    const bool shouldShow = !state.sceneId.startsWith(QStringLiteral("scene7b_dream_"))
        && state.shaderEffect == ShaderEffect::None
        && (shouldShowChuanGe || shouldShowLingling);

    if (!shouldShow) {
        m_characterPortraitLabel->hide();
        return;
    }

    QString portraitId;
    if (shouldShowLingling) {
        if (inLinglingScene6 || inColdOpenLingling) {
            portraitId = QString::fromUtf8(u8"鐏电伒/绔?png");
        } else if (inLinglingOpening) {
            portraitId = QString::fromUtf8(u8"鐏电伒/绔欏惉.png");
        } else if (inLinglingTopics) {
            const QString hidePlace = m_narrativeEngine->gameState().stringValue(QStringLiteral("hide_place"));
            portraitId = hidePlace == QStringLiteral("gym_back")
                ? QString::fromUtf8(u8"鐏电伒/鍧愬惉.png")
                : QString::fromUtf8(u8"鐏电伒/绔欏惉.png");
        } else if (inLinglingHug) {
            QString hugText = state.text;
            if (m_activeSceneId == QStringLiteral("scene7a_hug") && m_centerTextFrameMode
                && m_currentTextFrameIndex >= 0 && m_currentTextFrameIndex < m_textFrames.size()) {
                hugText = m_textFrames.at(m_currentTextFrameIndex);
            }
            if (hugText.contains(QString::fromUtf8(u8"抱住你"))) {
                m_linglingHugPortraitCompleted = true;
                m_linglingHugPortraitTimer->stop();
                portraitId = QString::fromUtf8(u8"鐏电伒/鎷ユ姳.png");
            } else if (hugText.contains(QString::fromUtf8(u8"鐏电伒浼告墜"))
                       || hugText.contains(QString::fromUtf8(u8"浼告墜"))) {
                m_linglingHugPortraitCompleted = false;
                m_linglingHugPortraitTimer->stop();
                portraitId = QString::fromUtf8(u8"鐏电伒/寮犲紑鍙屾墜.png");
            } else if (hugText.contains(QString::fromUtf8(u8"你终于彻底哭出来了"))) {
                m_linglingHugPortraitCompleted = true;
                m_linglingHugPortraitTimer->stop();
                portraitId = QString::fromUtf8(u8"鐏电伒/鎷ユ姳.png");
            } else {
                portraitId = QString::fromUtf8(u8"鐏电伒/绔欏惉.png");
            }
        } else {
            portraitId = QString::fromUtf8(u8"鐏电伒/鎷ユ姳.png");
        }
    } else if (inOfficeScene) {
        const bool isConflictSection = state.sceneId.startsWith(QStringLiteral("scene8_teacher_first_sting"))
            || state.sceneId.startsWith(QStringLiteral("scene8_after_sting_choice"))
            || state.sceneId.startsWith(QStringLiteral("scene8_explain_try"))
            || state.sceneId.startsWith(QStringLiteral("scene8_conflict_feedback_"));
        portraitId = isConflictSection
            ? QString::fromUtf8(u8"宸濆摜/鍧恄angry.png")
            : QString::fromUtf8(u8"宸濆摜/鍧恄peace.png");
    } else {
        portraitId = QString::fromUtf8(u8"宸濆摜/绔?png");
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
    if (m_floatingChoiceLayer && m_floatingChoiceLayer->isVisible()) {
        updateFloatingChoiceLayerGeometry();
    }
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
    const int dreamFrameDurationMs = dreamAutoFrameDurationMs(state.sceneId);
    if (dreamFrameDurationMs > 0) {
        durationMs = dreamFrameDurationMs;
    }
    if (durationMs <= 0) {
        durationMs = 900 + state.text.trimmed().size() * 85;
        durationMs = qMax(1100, qMin(durationMs, 4200));
    }
    if (state.sceneId == QStringLiteral("scene7a_hug")
        && state.text.contains(QString::fromUtf8(u8"灵灵伸手抱住你"))) {
        durationMs = qMax(durationMs, 1800);
    }
    if (state.sceneId == QStringLiteral("scene7a_hug")) {
        durationMs = 2000;
    }
    if (state.sceneId == QStringLiteral("scene10_song_performance")) {
        if (state.text.contains(QString::fromUtf8(u8"鐒跺悗浣犲紑濮嬪敱"))) {
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
    if (m_characterPortraitLabel && m_characterPortraitLabel->isVisible()) {
        m_characterPortraitLabel->raise();
    }
    if (m_headerLabel) {
        m_headerLabel->raise();
    }
    if (m_centerTextLabel) {
        m_centerTextLabel->raise();
    }
    if (m_dialoguePanel) {
        m_dialoguePanel->raise();
    }
    if (m_messageNotificationOverlay && m_messageNotificationOverlay->isVisible()) {
        m_messageNotificationOverlay->raise();
    }
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

QPixmap GameWindow::resolveTitleCoverPixmap() const
{
    const QString imagePath = resolveTitleCoverFilePath();
    if (imagePath.isEmpty()) {
        return {};
    }

    return QPixmap(imagePath);
}

#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "narrative/narrativeengine.h"
#include "ui/backgroundwidget.h"
#include "ui/interactiontypes.h"

#include <QHash>
#include <QList>
#include <QMainWindow>
#include <QPixmap>
#include <QString>
#include <QStringList>

class QElapsedTimer;
class DialoguePanel;
class FloatingChoiceLayer;
class AudioManager;
class AudioTestDialog;
class QGraphicsOpacityEffect;
class QLabel;
class QKeyEvent;
class QPropertyAnimation;
class QPixmap;
class QPushButton;
class QResizeEvent;
class ShaderToyWidget;
class QTimer;
struct NarrativeViewState;

class GameWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit GameWindow(QWidget *parent = nullptr);

    void setSpeaker(const QString &speaker);
    void clearSpeaker();
    void setText(const QString &text);
    void setContinueVisible(bool visible);
    void setDialogueVisible(bool visible);
    void setBackgroundStyle(BackgroundStyle style);
    void setInteractionItems(const InteractionItems &items);
    void setInteractionMode(InteractionMode mode);

    void showNarrationMode(const QString &text, bool showContinue);
    void showDialogueMode(const QString &speaker, const QString &text, bool showContinue);
    void showPerformanceMode(const QString &text);
    void showInteractionMode(const QString &speaker,
                             const QString &text,
                             InteractionMode mode,
                             const InteractionItems &items,
                             bool showContinue);

protected:
    void keyPressEvent(QKeyEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;

private slots:
    void advanceNarrative();
    void advanceAutoNarrative();
    void advanceCenterTextTypewriter();
    void stopAmbientAfterBgmLeadIn();
    void handleInteractionTriggered(const QString &id);
    void applyNarrativeState();
    void openAudioTestDialog();
    void completeLinglingHugPortrait();
    void handleContinueButtonClicked();
    void handleInteractionButtonClicked(const QString &id);
    void startPendingBgm();

private:
    void buildUi();
    void buildTitleScreen();
    void showTitleScreen();
    void updateTitleScreenGeometry();
    void startGameFromTitle();
    void finishTitleScreenTransition();
    void loadAudioManifest();
    void loadStoryContent();
    void syncAmbientAudio(const NarrativeViewState &state);
    void syncBgmAudio(const NarrativeViewState &state, bool sceneChanged);
    void playConfiguredSceneEntrySounds(const NarrativeViewState &state, bool sceneChanged);
    void playTextBlipIfNeeded(const QString &text);
    void playUiClickSound();
    void setHeaderText(const QString &text);
    void setCenterText(const QString &text);
    void clearCenterText();
    void startCenterTextTypewriter(const QString &text, int intervalMs = 42);
    void stopCenterTextTypewriter();
    void updateCenterTextAppearance();
    void setPagedText(const QString &text, bool showContinue);
    bool showNextTextFrame();
    void clearPagedText();
    void refreshPagedText();
    void updateDialoguePanelBounds(bool hasInteractions, int interactionCount = 0);
    void updateFloatingChoiceLayerGeometry();
    void updateCharacterPortrait(const NarrativeViewState &state);
    void updateCharacterPortraitGeometry(const QPixmap &portrait);
    void scheduleAutoAdvance(const NarrativeViewState &state);
    void setShaderEffect(ShaderEffect effect);
    QStringList buildCustomTextFrames(const QString &text) const;
    QPixmap resolveCharacterPortrait(const QString &characterId);
    QPixmap resolveBackgroundPixmap(BackgroundStyle style);
    QPixmap resolveTitleCoverPixmap() const;
    bool shouldSuppressAmbientForState(const NarrativeViewState &state) const;
    bool isMessageNotificationScene(const QString &sceneId) const;
    void showMessageNotificationMode(const QString &text);
    void resetMessageNotificationOverlay();
    void revealNextMessageNotification();
    void completeMessageNotificationSequence();
    void advanceMessageNotificationScene();
    void updateMessageNotificationGeometry();
    void layoutMessageNotificationRows();
    void updateDreamFadeGeometry();
    void clearDreamFade();
    void startDreamFade(qreal startOpacity, qreal endOpacity, int durationMs);
    void applyDreamPresentation(const NarrativeViewState &state, bool sceneChanged);
    void applyDreamShaderBinding(const NarrativeViewState &state);
    void setDreamShaderOverlayEnabled(bool enabled);
    void setDreamShaderOverlayFile(const QString &shaderFilePath);
    void setDreamShaderOverlayUseNoiseTexture(bool useNoiseTexture);
    void clearDreamShaderOverlay();
    QString displayHeaderText(const QString &text) const;
    QString displaySpeakerName(const QString &speaker) const;
    QString displayCenterText(const QString &text) const;
    bool isColdOpenCenterTextScene(const QString &sceneId) const;
    bool shouldUseFloatingChoiceLayer(InteractionMode mode, const InteractionItems &items) const;
    void showFloatingChoiceLayer(const InteractionItems &items);
    void hideFloatingChoiceLayer();

    BackgroundWidget *m_backgroundWidget;
    QWidget *m_dreamFadeOverlay;
    QGraphicsOpacityEffect *m_dreamFadeEffect;
    QPropertyAnimation *m_dreamFadeAnimation;
    ShaderToyWidget *m_shaderWidget;
    QLabel *m_characterPortraitLabel;
    DialoguePanel *m_dialoguePanel;
    FloatingChoiceLayer *m_floatingChoiceLayer;
    QLabel *m_centerTextLabel;
    QLabel *m_headerLabel;
    QWidget *m_titleScreenOverlay;
    QLabel *m_titleBackgroundLabel;
    QPushButton *m_titleStartButton;
    QGraphicsOpacityEffect *m_titleScreenEffect;
    QPropertyAnimation *m_titleScreenFadeAnimation;
    QWidget *m_messageNotificationOverlay;
    QWidget *m_messageNotificationCard;
    QWidget *m_messageNotificationMessagesWidget;
    QPushButton *m_messageNotificationContinueButton;
    AudioManager *m_audioManager;
    AudioTestDialog *m_audioTestDialog;
    NarrativeEngine *m_narrativeEngine;
    QTimer *m_autoAdvanceTimer;
    QTimer *m_bgmStartDelayTimer;
    QTimer *m_ambStopAfterBgmTimer;
    QTimer *m_centerTextTypewriterTimer;
    QTimer *m_messageNotificationTimer;
    QTimer *m_linglingHugPortraitTimer;
    QElapsedTimer *m_textBlipThrottleTimer;
    QHash<int, QPixmap> m_backgroundCache;
    QHash<QString, QPixmap> m_characterPortraitCache;
    QPixmap m_titleCoverPixmap;
    NarrativeViewState *m_lastNarrativeViewState;
    QString m_activeSceneId;
    QString m_pendingBgmAudioId;
    QString m_centerTextTarget;
    QString m_sourceText;
    QStringList m_textFrames;
    QStringList m_messageNotificationTexts;
    QList<QWidget *> m_messageNotificationBubbles;
    int m_currentTextFrameIndex;
    int m_centerTextVisibleCharacters;
    int m_messageNotificationIndex;
    bool m_centerTextFrameMode;
    bool m_scene8EnterPromptExpanded;
    bool m_linglingHugPortraitCompleted;
    bool m_messageNotificationAwaitingContinue;
    bool m_titleScreenActive;
    bool m_titleScreenTransitioning;
    qint64 m_lastTextBlipMs;
};

#endif // GAMEWINDOW_H

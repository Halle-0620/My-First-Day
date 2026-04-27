#ifndef GAMEWINDOW_H
#define GAMEWINDOW_H

#include "narrative/narrativeengine.h"
#include "ui/backgroundwidget.h"
#include "ui/interactiontypes.h"

#include <QMainWindow>
#include <QString>
#include <QStringList>

class DialoguePanel;
class QLabel;
class QKeyEvent;
class QTimer;

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

private slots:
    void advanceNarrative();
    void advanceAutoNarrative();
    void handleInteractionTriggered(const QString &id);
    void applyNarrativeState();

private:
    void buildUi();
    void loadStoryContent();
    void setHeaderText(const QString &text);
    void setCenterText(const QString &text);
    void clearCenterText();
    void scheduleAutoAdvance(const QString &text);

    BackgroundWidget *m_backgroundWidget;
    DialoguePanel *m_dialoguePanel;
    QLabel *m_centerTextLabel;
    QLabel *m_headerLabel;
    NarrativeEngine *m_narrativeEngine;
    QTimer *m_autoAdvanceTimer;
};

#endif // GAMEWINDOW_H

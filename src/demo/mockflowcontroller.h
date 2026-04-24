#ifndef MOCKFLOWCONTROLLER_H
#define MOCKFLOWCONTROLLER_H

#include <QSet>
#include <QString>

#include "../ui/backgroundwidget.h"
#include "../ui/interactiontypes.h"

struct DemoUiState
{
    QString header;
    QString speaker;
    QString text;
    BackgroundWidget::BackgroundStyle backgroundStyle = BackgroundWidget::BackgroundStyle::ClassroomDusk;
    bool showDialogue = true;
    bool showSpeaker = true;
    bool showContinue = true;
    InteractionMode interactionMode = InteractionMode::None;
    InteractionItems interactionItems;
};

class MockFlowController
{
public:
    MockFlowController();

    DemoUiState currentState() const;
    DemoUiState advance();
    DemoUiState triggerInteraction(const QString &id);

private:
    enum class Step {
        IntroNarration,
        StandardDialogue,
        ChoicePrompt,
        ChoiceFeedback,
        LocationPrompt,
        LocationFeedback,
        TopicPrompt,
        ActionPrompt,
        ActionFeedback,
        PerformanceBeat,
        ObjectPrompt,
        EndCard
    };

    DemoUiState stateForCurrentStep() const;
    bool allTopicsVisited() const;
    bool allObjectsVisited() const;
    QString topicFeedbackText(const QString &id) const;
    QString objectFeedbackText(const QString &id) const;
    InteractionItems buildTopicItems() const;
    InteractionItems buildObjectItems() const;

    Step m_currentStep;
    QString m_choiceResultId;
    QString m_selectedLocationId;
    QString m_selectedActionId;
    QString m_lastTopicId;
    QString m_lastObjectId;
    QSet<QString> m_visitedTopics;
    QSet<QString> m_visitedObjects;
};

#endif // MOCKFLOWCONTROLLER_H

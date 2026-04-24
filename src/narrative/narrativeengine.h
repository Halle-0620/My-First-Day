#ifndef NARRATIVEENGINE_H
#define NARRATIVEENGINE_H

#include "gamestate.h"
#include "scene.h"

#include <QList>
#include <QObject>
#include <QSet>
#include <QString>

struct NarrativeViewState
{
    QString sceneId;
    QString header;
    QString speaker;
    QString text;
    BackgroundStyle backgroundStyle = BackgroundStyle::ClassroomDusk;
    NarrativeDisplayMode displayMode = NarrativeDisplayMode::Narration;
    InteractionMode interactionMode = InteractionMode::None;
    InteractionItems interactionItems;
    bool showSpeaker = false;
    bool showContinue = false;
};

class NarrativeEngine : public QObject
{
    Q_OBJECT

public:
    explicit NarrativeEngine(QObject *parent = nullptr);

    void loadScenes(const NarrativeSceneMap &scenes, const QString &startSceneId);
    void start();
    void continueNarrative();
    void handleInteraction(const QString &interactionId);

    NarrativeViewState currentViewState() const;
    const GameState &gameState() const;
    bool hasCurrentScene() const;

signals:
    void stateChanged();

private:
    const NarrativeScene *currentScene() const;
    const NarrativeInteraction *findInteraction(const NarrativeScene &scene, const QString &interactionId) const;
    void enterScene(const QString &sceneId);
    void applyInteractionWrites(const NarrativeInteraction &interaction);
    QString resolvedText(const NarrativeScene &scene) const;
    QString resolveConditionalText(const QList<ConditionalTextVariant> &variants) const;
    QString resolvedSpeaker(const NarrativeScene &scene) const;
    QString resolvedNextSceneId(const NarrativeScene &scene) const;
    QString resolvedCompletionNextSceneId(const NarrativeScene &scene) const;
    BackgroundStyle resolvedBackgroundStyle(const NarrativeScene &scene) const;
    InteractionItems buildInteractionItems(const NarrativeScene &scene) const;
    bool allRequiredInteractionsVisited(const NarrativeScene &scene) const;

    NarrativeSceneMap m_scenes;
    QString m_startSceneId;
    QString m_currentSceneId;
    GameState m_gameState;
    QSet<QString> m_visitedInteractions;
    QString m_runtimeFeedbackText;
};

#endif // NARRATIVEENGINE_H

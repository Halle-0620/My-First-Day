#ifndef SCENE_H
#define SCENE_H

#include "../core/backgroundstyle.h"
#include "../core/interactiontypes.h"

#include <QHash>
#include <QList>
#include <QString>

#include <functional>

class GameState;

struct NarrativeCondition
{
    bool hasBoolCondition = false;
    QString boolKey;
    bool boolValue = false;

    bool hasStringCondition = false;
    QString stringKey;
    QString stringEquals;

    bool isEmpty() const;
    bool matches(const GameState &state) const;
};

struct ConditionalTextVariant
{
    NarrativeCondition condition;
    QString text;
};

struct ConditionalSceneLink
{
    NarrativeCondition condition;
    QString nextSceneId;
};

struct ConditionalBackgroundVariant
{
    NarrativeCondition condition;
    BackgroundStyle backgroundStyle = BackgroundStyle::ClassroomDusk;
};

enum class NarrativeDisplayMode {
    Narration,
    Dialogue,
    Performance,
    Interaction
};

struct NarrativeInteraction
{
    QString id;
    QString text;
    InteractionItemType type = InteractionItemType::Choice;
    QString nextSceneId;
    QString feedbackText;
    QList<QString> feedbackTextsByOrder;
    QList<QList<QString>> feedbackSequencesByOrder;
    QHash<QString, bool> boolWrites;
    QHash<QString, QString> stringWrites;
};

using SceneTextResolver = std::function<QString(const GameState &)>;
using SceneSpeakerResolver = std::function<QString(const GameState &)>;
using SceneIdResolver = std::function<QString(const GameState &)>;
using SceneBackgroundResolver = std::function<BackgroundStyle(const GameState &)>;
using SceneEnterHandler = std::function<void(GameState &)>;

struct NarrativeScene
{
    QString id;
    QString header;
    NarrativeDisplayMode displayMode = NarrativeDisplayMode::Narration;
    BackgroundStyle backgroundStyle = BackgroundStyle::ClassroomDusk;
    QString speaker;
    QString text;
    InteractionMode interactionMode = InteractionMode::None;
    QList<NarrativeInteraction> interactions;
    QString nextSceneId;
    QString completionText;
    QString completionNextSceneId;
    QList<QString> progressTexts;
    bool requiresAllInteractions = false;
    QHash<QString, bool> boolWrites;
    QHash<QString, QString> stringWrites;
    QList<ConditionalTextVariant> textVariants;
    QList<ConditionalTextVariant> completionTextVariants;
    QList<ConditionalSceneLink> nextVariants;
    QList<ConditionalSceneLink> completionNextVariants;
    QList<ConditionalBackgroundVariant> backgroundVariants;
    SceneTextResolver textResolver;
    SceneTextResolver completionTextResolver;
    SceneSpeakerResolver speakerResolver;
    SceneIdResolver nextSceneResolver;
    SceneIdResolver completionNextSceneResolver;
    SceneBackgroundResolver backgroundResolver;
    SceneEnterHandler onEnter;
};

using NarrativeSceneMap = QHash<QString, NarrativeScene>;

#endif // SCENE_H

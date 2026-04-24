#include "narrativeengine.h"

NarrativeEngine::NarrativeEngine(QObject *parent)
    : QObject(parent)
{
}

void NarrativeEngine::loadScenes(const NarrativeSceneMap &scenes, const QString &startSceneId)
{
    m_scenes = scenes;
    m_startSceneId = startSceneId;
    m_currentSceneId.clear();
    m_gameState.clear();
    m_visitedInteractions.clear();
    m_runtimeFeedbackText.clear();
}

void NarrativeEngine::start()
{
    if (!m_startSceneId.isEmpty()) {
        enterScene(m_startSceneId);
    }
}

void NarrativeEngine::continueNarrative()
{
    const NarrativeScene *scene = currentScene();
    if (!scene) {
        return;
    }

    QString nextSceneId;

    if (scene->requiresAllInteractions) {
        if (!allRequiredInteractionsVisited(*scene)) {
            return;
        }

        nextSceneId = resolvedCompletionNextSceneId(*scene);
    } else {
        nextSceneId = resolvedNextSceneId(*scene);
    }

    if (!nextSceneId.isEmpty()) {
        enterScene(nextSceneId);
    }
}

void NarrativeEngine::handleInteraction(const QString &interactionId)
{
    const NarrativeScene *scene = currentScene();
    if (!scene) {
        return;
    }

    const NarrativeInteraction *interaction = findInteraction(*scene, interactionId);
    if (!interaction) {
        return;
    }

    if (scene->requiresAllInteractions && m_visitedInteractions.contains(interactionId)) {
        return;
    }

    applyInteractionWrites(*interaction);

    if (scene->requiresAllInteractions) {
        m_visitedInteractions.insert(interactionId);
        m_runtimeFeedbackText = interaction->feedbackText;
        emit stateChanged();
        return;
    }

    if (!interaction->nextSceneId.isEmpty()) {
        enterScene(interaction->nextSceneId);
        return;
    }

    if (!interaction->feedbackText.isEmpty()) {
        m_runtimeFeedbackText = interaction->feedbackText;
        emit stateChanged();
    }
}

NarrativeViewState NarrativeEngine::currentViewState() const
{
    NarrativeViewState viewState;

    const NarrativeScene *scene = currentScene();
    if (!scene) {
        return viewState;
    }

    viewState.sceneId = scene->id;
    viewState.header = scene->header.isEmpty() ? scene->id : scene->header;
    viewState.speaker = resolvedSpeaker(*scene);
    viewState.text = resolvedText(*scene);
    viewState.backgroundStyle = resolvedBackgroundStyle(*scene);
    viewState.displayMode = scene->displayMode;
    viewState.interactionMode = scene->interactionMode;
    viewState.interactionItems = buildInteractionItems(*scene);
    viewState.showSpeaker = !viewState.speaker.trimmed().isEmpty();

    if (scene->displayMode == NarrativeDisplayMode::Interaction) {
        if (scene->requiresAllInteractions) {
            viewState.showContinue = allRequiredInteractionsVisited(*scene)
                && !resolvedCompletionNextSceneId(*scene).isEmpty();
        } else {
            viewState.showContinue = false;
        }
    } else {
        viewState.showContinue = !resolvedNextSceneId(*scene).isEmpty();
    }

    return viewState;
}

const GameState &NarrativeEngine::gameState() const
{
    return m_gameState;
}

bool NarrativeEngine::hasCurrentScene() const
{
    return currentScene() != nullptr;
}

const NarrativeScene *NarrativeEngine::currentScene() const
{
    auto it = m_scenes.constFind(m_currentSceneId);
    if (it == m_scenes.cend()) {
        return nullptr;
    }

    return &it.value();
}

const NarrativeInteraction *NarrativeEngine::findInteraction(const NarrativeScene &scene,
                                                            const QString &interactionId) const
{
    for (const NarrativeInteraction &interaction : scene.interactions) {
        if (interaction.id == interactionId) {
            return &interaction;
        }
    }

    return nullptr;
}

void NarrativeEngine::enterScene(const QString &sceneId)
{
    if (!m_scenes.contains(sceneId)) {
        return;
    }

    m_currentSceneId = sceneId;
    m_visitedInteractions.clear();
    m_runtimeFeedbackText.clear();

    NarrativeScene &scene = m_scenes[m_currentSceneId];

    for (auto it = scene.boolWrites.cbegin(); it != scene.boolWrites.cend(); ++it) {
        m_gameState.setBool(it.key(), it.value());
    }

    for (auto it = scene.stringWrites.cbegin(); it != scene.stringWrites.cend(); ++it) {
        m_gameState.setString(it.key(), it.value());
    }

    if (scene.onEnter) {
        scene.onEnter(m_gameState);
    }

    emit stateChanged();
}

void NarrativeEngine::applyInteractionWrites(const NarrativeInteraction &interaction)
{
    for (auto it = interaction.boolWrites.cbegin(); it != interaction.boolWrites.cend(); ++it) {
        m_gameState.setBool(it.key(), it.value());
    }

    for (auto it = interaction.stringWrites.cbegin(); it != interaction.stringWrites.cend(); ++it) {
        m_gameState.setString(it.key(), it.value());
    }
}

QString NarrativeEngine::resolvedText(const NarrativeScene &scene) const
{
    if (scene.requiresAllInteractions && allRequiredInteractionsVisited(scene)) {
        const QString completionVariantText = resolveConditionalText(scene.completionTextVariants);
        if (!completionVariantText.isEmpty()) {
            return completionVariantText;
        }

        if (scene.completionTextResolver) {
            return scene.completionTextResolver(m_gameState);
        }

        if (!scene.completionText.isEmpty()) {
            return scene.completionText;
        }
    }

    if (!m_runtimeFeedbackText.isEmpty()) {
        return m_runtimeFeedbackText;
    }

    const QString variantText = resolveConditionalText(scene.textVariants);
    if (!variantText.isEmpty()) {
        return variantText;
    }

    if (scene.textResolver) {
        return scene.textResolver(m_gameState);
    }

    return scene.text;
}

QString NarrativeEngine::resolveConditionalText(const QList<ConditionalTextVariant> &variants) const
{
    for (const ConditionalTextVariant &variant : variants) {
        if (!variant.condition.isEmpty() && variant.condition.matches(m_gameState)) {
            return variant.text;
        }
    }

    return {};
}

QString NarrativeEngine::resolvedSpeaker(const NarrativeScene &scene) const
{
    if (scene.speakerResolver) {
        return scene.speakerResolver(m_gameState);
    }

    return scene.speaker;
}

QString NarrativeEngine::resolvedNextSceneId(const NarrativeScene &scene) const
{
    for (const ConditionalSceneLink &variant : scene.nextVariants) {
        if (!variant.condition.isEmpty() && variant.condition.matches(m_gameState)) {
            return variant.nextSceneId;
        }
    }

    if (scene.nextSceneResolver) {
        return scene.nextSceneResolver(m_gameState);
    }

    return scene.nextSceneId;
}

QString NarrativeEngine::resolvedCompletionNextSceneId(const NarrativeScene &scene) const
{
    for (const ConditionalSceneLink &variant : scene.completionNextVariants) {
        if (!variant.condition.isEmpty() && variant.condition.matches(m_gameState)) {
            return variant.nextSceneId;
        }
    }

    if (scene.completionNextSceneResolver) {
        return scene.completionNextSceneResolver(m_gameState);
    }

    return scene.completionNextSceneId;
}

BackgroundStyle NarrativeEngine::resolvedBackgroundStyle(const NarrativeScene &scene) const
{
    for (const ConditionalBackgroundVariant &variant : scene.backgroundVariants) {
        if (!variant.condition.isEmpty() && variant.condition.matches(m_gameState)) {
            return variant.backgroundStyle;
        }
    }

    if (scene.backgroundResolver) {
        return scene.backgroundResolver(m_gameState);
    }

    return scene.backgroundStyle;
}

InteractionItems NarrativeEngine::buildInteractionItems(const NarrativeScene &scene) const
{
    InteractionItems items;
    items.reserve(scene.interactions.size());

    for (const NarrativeInteraction &interaction : scene.interactions) {
        const bool visited = scene.requiresAllInteractions && m_visitedInteractions.contains(interaction.id);

        items.append({
            interaction.id,
            interaction.text,
            interaction.type,
            !visited,
            visited,
            false
        });
    }

    return items;
}

bool NarrativeEngine::allRequiredInteractionsVisited(const NarrativeScene &scene) const
{
    if (!scene.requiresAllInteractions) {
        return true;
    }

    for (const NarrativeInteraction &interaction : scene.interactions) {
        if (!m_visitedInteractions.contains(interaction.id)) {
            return false;
        }
    }

    return true;
}

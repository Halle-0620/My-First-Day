#include "narrativeengine.h"

#include <QStringList>

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
    m_runtimeShaderEffect = ShaderEffect::None;
    m_runtimeShaderDurationMs = 0;
    m_runtimeFeedbackText.clear();
    m_runtimeFeedbackFrames.clear();
    m_runtimeFeedbackIndex = -1;
    m_sceneAutoFrames.clear();
    m_sceneAutoIndex = -1;
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

    if (hasActiveRuntimeFeedback()) {
        if (m_runtimeFeedbackIndex + 1 < m_runtimeFeedbackFrames.size()) {
            ++m_runtimeFeedbackIndex;
            emit stateChanged();
            return;
        }

        m_runtimeFeedbackFrames.clear();
        m_runtimeFeedbackIndex = -1;
        m_runtimeFeedbackText.clear();

        if (scene->requiresAllInteractions && allRequiredInteractionsVisited(*scene)) {
            nextSceneId = resolvedCompletionNextSceneId(*scene);
            if (!nextSceneId.isEmpty()) {
                enterScene(nextSceneId);
                return;
            }
        }

        emit stateChanged();
        return;
    }

    if (hasActiveRuntimeShader()) {
        m_runtimeShaderEffect = ShaderEffect::None;
        m_runtimeShaderDurationMs = 0;
        emit stateChanged();
        return;
    }

    if (hasActiveSceneAutoFrame()) {
        if (m_sceneAutoIndex + 1 < m_sceneAutoFrames.size()) {
            ++m_sceneAutoIndex;
            emit stateChanged();
            return;
        }

        nextSceneId = resolvedNextSceneId(*scene);
        if (!nextSceneId.isEmpty()) {
            enterScene(nextSceneId);
        }
        return;
    }

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
        const int feedbackOrder = m_visitedInteractions.size();
        m_visitedInteractions.insert(interactionId);
        if (interaction->shaderEffect != ShaderEffect::None && interaction->shaderDurationMs > 0) {
            m_runtimeShaderEffect = interaction->shaderEffect;
            m_runtimeShaderDurationMs = interaction->shaderDurationMs;
            emit stateChanged();
            return;
        }
        if (interaction->feedbackSequencesByOrder.size() == 1
            && !interaction->feedbackSequencesByOrder.first().isEmpty()) {
            m_runtimeFeedbackFrames = interaction->feedbackSequencesByOrder.first();
            m_runtimeFeedbackIndex = 0;
            m_runtimeFeedbackText = m_runtimeFeedbackFrames.first();
        } else if (feedbackOrder < interaction->feedbackSequencesByOrder.size()
            && !interaction->feedbackSequencesByOrder.at(feedbackOrder).isEmpty()) {
            m_runtimeFeedbackFrames = interaction->feedbackSequencesByOrder.at(feedbackOrder);
            m_runtimeFeedbackIndex = 0;
            m_runtimeFeedbackText = m_runtimeFeedbackFrames.first();
        } else if (feedbackOrder < interaction->feedbackTextsByOrder.size()
            && !interaction->feedbackTextsByOrder.at(feedbackOrder).isEmpty()) {
            m_runtimeFeedbackFrames = splitTextFrames(interaction->feedbackTextsByOrder.at(feedbackOrder));
            m_runtimeFeedbackIndex = m_runtimeFeedbackFrames.isEmpty() ? -1 : 0;
            m_runtimeFeedbackText = m_runtimeFeedbackFrames.isEmpty() ? QString() : m_runtimeFeedbackFrames.first();
        } else {
            m_runtimeFeedbackFrames = splitTextFrames(interaction->feedbackText);
            m_runtimeFeedbackIndex = m_runtimeFeedbackFrames.isEmpty() ? -1 : 0;
            m_runtimeFeedbackText = m_runtimeFeedbackFrames.isEmpty() ? QString() : m_runtimeFeedbackFrames.first();
        }

        if (!hasActiveRuntimeFeedback() && allRequiredInteractionsVisited(*scene)) {
            const QString nextSceneId = resolvedCompletionNextSceneId(*scene);
            if (!nextSceneId.isEmpty()) {
                enterScene(nextSceneId);
                return;
            }
        }

        emit stateChanged();
        return;
    }

    if (interaction->shaderEffect != ShaderEffect::None && interaction->shaderDurationMs > 0) {
        m_runtimeShaderEffect = interaction->shaderEffect;
        m_runtimeShaderDurationMs = interaction->shaderDurationMs;
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
    viewState.shaderEffect = scene->shaderEffect;
    viewState.showSpeaker = !viewState.speaker.trimmed().isEmpty();
    viewState.autoAdvanceDurationMs = scene->autoAdvanceDurationMs;

    if (hasActiveRuntimeShader()) {
        viewState.displayMode = NarrativeDisplayMode::Performance;
        viewState.speaker.clear();
        viewState.text.clear();
        viewState.interactionMode = InteractionMode::None;
        viewState.interactionItems.clear();
        viewState.shaderEffect = m_runtimeShaderEffect;
        viewState.showSpeaker = false;
        viewState.showContinue = false;
        viewState.autoAdvance = true;
        viewState.autoAdvanceDurationMs = m_runtimeShaderDurationMs;
        return viewState;
    }

    if (hasActiveRuntimeFeedback()) {
        QString frameText = m_runtimeFeedbackFrames.at(m_runtimeFeedbackIndex);
        QString frameSpeaker;
        if (extractSpeakerPrefix(&frameText, &frameSpeaker)) {
            viewState.displayMode = NarrativeDisplayMode::Dialogue;
            viewState.speaker = frameSpeaker;
            viewState.showSpeaker = true;
        } else {
            viewState.displayMode = NarrativeDisplayMode::Performance;
            viewState.speaker.clear();
            viewState.showSpeaker = false;
        }
        viewState.text = frameText;
        viewState.interactionMode = InteractionMode::None;
        viewState.interactionItems.clear();
        viewState.showContinue = false;
        viewState.autoAdvance = true;
        viewState.autoAdvanceDurationMs = 0;
        return viewState;
    }

    if (hasActiveSceneAutoFrame()) {
        QString frameText = m_sceneAutoFrames.at(m_sceneAutoIndex);
        QString frameSpeaker;
        if (extractSpeakerPrefix(&frameText, &frameSpeaker)) {
            viewState.displayMode = NarrativeDisplayMode::Dialogue;
            viewState.speaker = frameSpeaker;
            viewState.showSpeaker = true;
        } else {
            viewState.displayMode = NarrativeDisplayMode::Performance;
            viewState.speaker.clear();
            viewState.showSpeaker = false;
        }
        viewState.text = frameText;
        viewState.interactionMode = InteractionMode::None;
        viewState.interactionItems.clear();
        viewState.showContinue = false;
        viewState.autoAdvance = !resolvedNextSceneId(*scene).isEmpty()
            || m_sceneAutoIndex + 1 < m_sceneAutoFrames.size();
        viewState.autoAdvanceDurationMs = 0;
        return viewState;
    }

    if (scene->displayMode == NarrativeDisplayMode::Interaction) {
        if (scene->requiresAllInteractions) {
            viewState.showContinue = allRequiredInteractionsVisited(*scene)
                && !resolvedCompletionNextSceneId(*scene).isEmpty();
        } else {
            viewState.showContinue = false;
        }
    } else if (scene->displayMode == NarrativeDisplayMode::Performance) {
        viewState.showContinue = false;
    } else {
        viewState.showContinue = !resolvedNextSceneId(*scene).isEmpty();
    }

    if (scene->autoAdvanceDurationMs > 0 && !resolvedNextSceneId(*scene).isEmpty()) {
        viewState.autoAdvance = true;
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
    m_runtimeShaderEffect = ShaderEffect::None;
    m_runtimeShaderDurationMs = 0;
    m_runtimeFeedbackText.clear();
    m_runtimeFeedbackFrames.clear();
    m_runtimeFeedbackIndex = -1;
    m_sceneAutoFrames.clear();
    m_sceneAutoIndex = -1;

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

    if (scene.displayMode == NarrativeDisplayMode::Performance) {
        m_sceneAutoFrames = splitTextFrames(resolvedPrimaryText(scene));
        m_sceneAutoIndex = m_sceneAutoFrames.isEmpty() ? -1 : 0;
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
    if (hasActiveRuntimeFeedback()) {
        return m_runtimeFeedbackFrames.at(m_runtimeFeedbackIndex);
    }

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

    if (scene.requiresAllInteractions && !allRequiredInteractionsVisited(scene)) {
        const int visitedCount = m_visitedInteractions.size();
        if (visitedCount > 0 && visitedCount <= scene.progressTexts.size()
            && !scene.progressTexts.at(visitedCount - 1).isEmpty()) {
            return scene.progressTexts.at(visitedCount - 1);
        }
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

bool NarrativeEngine::hasActiveRuntimeShader() const
{
    return m_runtimeShaderEffect != ShaderEffect::None && m_runtimeShaderDurationMs > 0;
}

bool NarrativeEngine::hasActiveRuntimeFeedback() const
{
    return m_runtimeFeedbackIndex >= 0 && m_runtimeFeedbackIndex < m_runtimeFeedbackFrames.size();
}

bool NarrativeEngine::hasActiveSceneAutoFrame() const
{
    return m_sceneAutoIndex >= 0 && m_sceneAutoIndex < m_sceneAutoFrames.size();
}

QString NarrativeEngine::resolvedPrimaryText(const NarrativeScene &scene) const
{
    const QString variantText = resolveConditionalText(scene.textVariants);
    if (!variantText.isEmpty()) {
        return variantText;
    }

    if (scene.textResolver) {
        return scene.textResolver(m_gameState);
    }

    return scene.text;
}

QList<QString> NarrativeEngine::splitTextFrames(const QString &text) const
{
    QList<QString> frames;
    const QString normalized = QString(text).replace(QStringLiteral("\r\n"), QStringLiteral("\n"));

    for (const QString &block : normalized.split(QStringLiteral("\n\n"), Qt::SkipEmptyParts)) {
        const QString trimmed = block.trimmed();
        if (!trimmed.isEmpty()) {
            frames.append(trimmed);
        }
    }

    if (frames.isEmpty() && !text.trimmed().isEmpty()) {
        frames.append(text.trimmed());
    }

    return frames;
}

bool NarrativeEngine::extractSpeakerPrefix(QString *text, QString *speaker) const
{
    static const QStringList speakers = {
        QStringLiteral("你"),
        QStringLiteral("灵灵"),
        QStringLiteral("川哥"),
        QStringLiteral("富豪")
    };

    if (!text || !speaker) {
        return false;
    }

    for (const QString &candidate : speakers) {
        const QString prefix = candidate + QStringLiteral("：");
        if (text->startsWith(prefix)) {
            *speaker = candidate;
            *text = text->mid(prefix.size()).trimmed();
            return true;
        }
    }

    return false;
}

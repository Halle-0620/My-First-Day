#include "storyloader.h"

#include <QFile>
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>

namespace {

QString requireString(const QJsonObject &object, const QString &key, QString *errorMessage)
{
    if (!object.contains(key) || !object.value(key).isString()) {
        if (errorMessage) {
            *errorMessage = QStringLiteral("Missing or invalid string field: %1").arg(key);
        }
        return {};
    }

    return object.value(key).toString();
}

BackgroundStyle parseBackgroundStyle(const QString &value, bool *ok)
{
    if (value == QStringLiteral("classroom_dusk")) {
        *ok = true;
        return BackgroundStyle::ClassroomDusk;
    }
    if (value == QStringLiteral("quiet_hallway")) {
        *ok = true;
        return BackgroundStyle::QuietHallway;
    }
    if (value == QStringLiteral("choice_focus")) {
        *ok = true;
        return BackgroundStyle::ChoiceFocus;
    }
    if (value == QStringLiteral("soft_narration")) {
        *ok = true;
        return BackgroundStyle::SoftNarration;
    }
    if (value == QStringLiteral("night_rain")) {
        *ok = true;
        return BackgroundStyle::NightRain;
    }
    if (value == QStringLiteral("ending_glow")) {
        *ok = true;
        return BackgroundStyle::EndingGlow;
    }
    if (value == QStringLiteral("ending_black")) {
        *ok = true;
        return BackgroundStyle::EndingBlack;
    }
    if (value == QStringLiteral("dream_drift")) {
        *ok = true;
        return BackgroundStyle::DreamDrift;
    }

    *ok = false;
    return BackgroundStyle::ClassroomDusk;
}

NarrativeDisplayMode parseDisplayMode(const QString &value, bool *ok)
{
    if (value == QStringLiteral("narration")) {
        *ok = true;
        return NarrativeDisplayMode::Narration;
    }
    if (value == QStringLiteral("dialogue")) {
        *ok = true;
        return NarrativeDisplayMode::Dialogue;
    }
    if (value == QStringLiteral("performance")) {
        *ok = true;
        return NarrativeDisplayMode::Performance;
    }
    if (value == QStringLiteral("interaction")) {
        *ok = true;
        return NarrativeDisplayMode::Interaction;
    }

    *ok = false;
    return NarrativeDisplayMode::Narration;
}

InteractionMode parseInteractionMode(const QString &value, bool *ok)
{
    if (value == QStringLiteral("none")) {
        *ok = true;
        return InteractionMode::None;
    }
    if (value == QStringLiteral("choice")) {
        *ok = true;
        return InteractionMode::Choice;
    }
    if (value == QStringLiteral("hotspot")) {
        *ok = true;
        return InteractionMode::Hotspot;
    }
    if (value == QStringLiteral("action")) {
        *ok = true;
        return InteractionMode::Action;
    }
    if (value == QStringLiteral("topic")) {
        *ok = true;
        return InteractionMode::Topic;
    }
    if (value == QStringLiteral("object")) {
        *ok = true;
        return InteractionMode::Object;
    }

    *ok = false;
    return InteractionMode::None;
}

InteractionItemType parseInteractionItemType(const QString &value, bool *ok)
{
    if (value == QStringLiteral("choice")) {
        *ok = true;
        return InteractionItemType::Choice;
    }
    if (value == QStringLiteral("hotspot")) {
        *ok = true;
        return InteractionItemType::Hotspot;
    }
    if (value == QStringLiteral("action")) {
        *ok = true;
        return InteractionItemType::Action;
    }
    if (value == QStringLiteral("topic")) {
        *ok = true;
        return InteractionItemType::Topic;
    }
    if (value == QStringLiteral("object")) {
        *ok = true;
        return InteractionItemType::Object;
    }

    *ok = false;
    return InteractionItemType::Choice;
}

bool parseCondition(const QJsonObject &object, NarrativeCondition *condition, QString *errorMessage)
{
    if (object.contains(QStringLiteral("if_bool"))) {
        const QJsonValue value = object.value(QStringLiteral("if_bool"));
        if (!value.isObject()) {
            *errorMessage = QStringLiteral("Field if_bool must be an object");
            return false;
        }

        const QJsonObject conditionObject = value.toObject();
        const QString key = requireString(conditionObject, QStringLiteral("key"), errorMessage);
        if (key.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        if (!conditionObject.contains(QStringLiteral("value")) || !conditionObject.value(QStringLiteral("value")).isBool()) {
            *errorMessage = QStringLiteral("Field if_bool.value must be a bool");
            return false;
        }

        condition->hasBoolCondition = true;
        condition->boolKey = key;
        condition->boolValue = conditionObject.value(QStringLiteral("value")).toBool();
    }

    if (object.contains(QStringLiteral("if_string_equals"))) {
        const QJsonValue value = object.value(QStringLiteral("if_string_equals"));
        if (!value.isObject()) {
            *errorMessage = QStringLiteral("Field if_string_equals must be an object");
            return false;
        }

        const QJsonObject conditionObject = value.toObject();
        const QString key = requireString(conditionObject, QStringLiteral("key"), errorMessage);
        if (key.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        const QString stringValue = requireString(conditionObject, QStringLiteral("value"), errorMessage);
        if (stringValue.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        condition->hasStringCondition = true;
        condition->stringKey = key;
        condition->stringEquals = stringValue;
    }

    return true;
}

bool parseBoolWrites(const QJsonObject &object,
                     const QString &key,
                     QHash<QString, bool> *writes,
                     QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }

    const QJsonValue value = object.value(key);
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Field %1 must be an object").arg(key);
        return false;
    }

    const QJsonObject writesObject = value.toObject();
    for (auto it = writesObject.begin(); it != writesObject.end(); ++it) {
        if (!it.value().isBool()) {
            *errorMessage = QStringLiteral("Value for %1.%2 must be a bool").arg(key, it.key());
            return false;
        }
        writes->insert(it.key(), it.value().toBool());
    }

    return true;
}

bool parseStringWrites(const QJsonObject &object,
                       const QString &key,
                       QHash<QString, QString> *writes,
                       QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }

    const QJsonValue value = object.value(key);
    if (!value.isObject()) {
        *errorMessage = QStringLiteral("Field %1 must be an object").arg(key);
        return false;
    }

    const QJsonObject writesObject = value.toObject();
    for (auto it = writesObject.begin(); it != writesObject.end(); ++it) {
        if (!it.value().isString()) {
            *errorMessage = QStringLiteral("Value for %1.%2 must be a string").arg(key, it.key());
            return false;
        }
        writes->insert(it.key(), it.value().toString());
    }

    return true;
}

bool parseStringList(const QJsonObject &object,
                     const QString &key,
                     QList<QString> *target,
                     QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }

    const QJsonValue value = object.value(key);
    if (!value.isArray()) {
        *errorMessage = QStringLiteral("Field %1 must be an array").arg(key);
        return false;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue &entryValue : array) {
        if (!entryValue.isString()) {
            *errorMessage = QStringLiteral("Entries in %1 must be strings").arg(key);
            return false;
        }
        target->append(entryValue.toString());
    }

    return true;
}

bool parseStringListList(const QJsonObject &object,
                         const QString &key,
                         QList<QList<QString>> *target,
                         QString *errorMessage)
{
    if (!object.contains(key)) {
        return true;
    }

    const QJsonValue value = object.value(key);
    if (!value.isArray()) {
        *errorMessage = QStringLiteral("Field %1 must be an array").arg(key);
        return false;
    }

    const QJsonArray outerArray = value.toArray();
    for (const QJsonValue &innerValue : outerArray) {
        if (!innerValue.isArray()) {
            *errorMessage = QStringLiteral("Entries in %1 must be arrays").arg(key);
            return false;
        }

        QList<QString> innerList;
        const QJsonArray innerArray = innerValue.toArray();
        for (const QJsonValue &entryValue : innerArray) {
            if (!entryValue.isString()) {
                *errorMessage = QStringLiteral("Nested entries in %1 must be strings").arg(key);
                return false;
            }
            innerList.append(entryValue.toString());
        }

        target->append(innerList);
    }

    return true;
}

bool parseStringListWithFallback(const QJsonObject &object,
                                 const QString &primaryKey,
                                 const QString &fallbackKey,
                                 QList<QString> *target,
                                 QString *errorMessage)
{
    if (object.contains(primaryKey)) {
        return parseStringList(object, primaryKey, target, errorMessage);
    }
    if (object.contains(fallbackKey)) {
        return parseStringList(object, fallbackKey, target, errorMessage);
    }
    return true;
}

bool parseStringListListWithFallback(const QJsonObject &object,
                                     const QString &primaryKey,
                                     const QString &fallbackKey,
                                     QList<QList<QString>> *target,
                                     QString *errorMessage)
{
    if (object.contains(primaryKey)) {
        return parseStringListList(object, primaryKey, target, errorMessage);
    }
    if (object.contains(fallbackKey)) {
        return parseStringListList(object, fallbackKey, target, errorMessage);
    }
    return true;
}

bool parseConditionalTexts(const QJsonObject &sceneObject,
                           const QString &key,
                           QList<ConditionalTextVariant> *variants,
                           QString *errorMessage)
{
    if (!sceneObject.contains(key)) {
        return true;
    }

    const QJsonValue value = sceneObject.value(key);
    if (!value.isArray()) {
        *errorMessage = QStringLiteral("Field %1 must be an array").arg(key);
        return false;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue &entryValue : array) {
        if (!entryValue.isObject()) {
            *errorMessage = QStringLiteral("Entries in %1 must be objects").arg(key);
            return false;
        }

        const QJsonObject entryObject = entryValue.toObject();
        ConditionalTextVariant variant;

        if (!parseCondition(entryObject, &variant.condition, errorMessage)) {
            return false;
        }

        variant.text = requireString(entryObject, QStringLiteral("text"), errorMessage);
        if (variant.text.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        variants->append(variant);
    }

    return true;
}

bool parseConditionalSceneLinks(const QJsonObject &sceneObject,
                                const QString &key,
                                QList<ConditionalSceneLink> *variants,
                                QString *errorMessage)
{
    if (!sceneObject.contains(key)) {
        return true;
    }

    const QJsonValue value = sceneObject.value(key);
    if (!value.isArray()) {
        *errorMessage = QStringLiteral("Field %1 must be an array").arg(key);
        return false;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue &entryValue : array) {
        if (!entryValue.isObject()) {
            *errorMessage = QStringLiteral("Entries in %1 must be objects").arg(key);
            return false;
        }

        const QJsonObject entryObject = entryValue.toObject();
        ConditionalSceneLink variant;

        if (!parseCondition(entryObject, &variant.condition, errorMessage)) {
            return false;
        }

        variant.nextSceneId = requireString(entryObject, QStringLiteral("next"), errorMessage);
        if (variant.nextSceneId.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        variants->append(variant);
    }

    return true;
}

bool parseConditionalBackgrounds(const QJsonObject &sceneObject,
                                 const QString &key,
                                 QList<ConditionalBackgroundVariant> *variants,
                                 QString *errorMessage)
{
    if (!sceneObject.contains(key)) {
        return true;
    }

    const QJsonValue value = sceneObject.value(key);
    if (!value.isArray()) {
        *errorMessage = QStringLiteral("Field %1 must be an array").arg(key);
        return false;
    }

    const QJsonArray array = value.toArray();
    for (const QJsonValue &entryValue : array) {
        if (!entryValue.isObject()) {
            *errorMessage = QStringLiteral("Entries in %1 must be objects").arg(key);
            return false;
        }

        const QJsonObject entryObject = entryValue.toObject();
        ConditionalBackgroundVariant variant;

        if (!parseCondition(entryObject, &variant.condition, errorMessage)) {
            return false;
        }

        const QString backgroundString = requireString(entryObject, QStringLiteral("background"), errorMessage);
        if (backgroundString.isEmpty() && !errorMessage->isEmpty()) {
            return false;
        }

        bool ok = false;
        variant.backgroundStyle = parseBackgroundStyle(backgroundString, &ok);
        if (!ok) {
            *errorMessage = QStringLiteral("Unknown background style: %1").arg(backgroundString);
            return false;
        }

        variants->append(variant);
    }

    return true;
}

bool parseInteraction(const QJsonObject &interactionObject,
                      NarrativeInteraction *interaction,
                      QString *errorMessage)
{
    interaction->id = requireString(interactionObject, QStringLiteral("id"), errorMessage);
    if (interaction->id.isEmpty() && !errorMessage->isEmpty()) {
        return false;
    }

    interaction->text = requireString(interactionObject, QStringLiteral("text"), errorMessage);
    if (interaction->text.isEmpty() && !errorMessage->isEmpty()) {
        return false;
    }

    const QString typeString = requireString(interactionObject, QStringLiteral("type"), errorMessage);
    if (typeString.isEmpty() && !errorMessage->isEmpty()) {
        return false;
    }

    bool ok = false;
    interaction->type = parseInteractionItemType(typeString, &ok);
    if (!ok) {
        *errorMessage = QStringLiteral("Unknown interaction type: %1").arg(typeString);
        return false;
    }

    if (interactionObject.contains(QStringLiteral("next"))) {
        if (!interactionObject.value(QStringLiteral("next")).isString()) {
            *errorMessage = QStringLiteral("Field interactions.next must be a string");
            return false;
        }
        interaction->nextSceneId = interactionObject.value(QStringLiteral("next")).toString();
    }

    if (interactionObject.contains(QStringLiteral("feedback_text"))) {
        if (!interactionObject.value(QStringLiteral("feedback_text")).isString()) {
            *errorMessage = QStringLiteral("Field feedback_text must be a string");
            return false;
        }
        interaction->feedbackText = interactionObject.value(QStringLiteral("feedback_text")).toString();
    }

    if (!parseStringListWithFallback(interactionObject,
                                     QStringLiteral("feedback_text_by_order"),
                                     QStringLiteral("feedback_texts_by_order"),
                                     &interaction->feedbackTextsByOrder,
                                     errorMessage)) {
        return false;
    }

    if (!parseStringListListWithFallback(interactionObject,
                                         QStringLiteral("feedback_sequence_by_order"),
                                         QStringLiteral("feedback_sequences_by_order"),
                                         &interaction->feedbackSequencesByOrder,
                                         errorMessage)) {
        return false;
    }

    if (!parseBoolWrites(interactionObject, QStringLiteral("set_bool"), &interaction->boolWrites, errorMessage)) {
        return false;
    }

    if (!parseStringWrites(interactionObject, QStringLiteral("set_string"), &interaction->stringWrites, errorMessage)) {
        return false;
    }

    return true;
}

bool validateReferences(const StoryLoadResult &result, QString *errorMessage)
{
    for (auto sceneIt = result.scenes.cbegin(); sceneIt != result.scenes.cend(); ++sceneIt) {
        const NarrativeScene &scene = sceneIt.value();

        auto ensureSceneExists = [&](const QString &sceneId, const QString &label) -> bool {
            if (!sceneId.isEmpty() && !result.scenes.contains(sceneId)) {
                *errorMessage = QStringLiteral("Scene '%1' references missing target '%2' in %3")
                    .arg(scene.id, sceneId, label);
                return false;
            }
            return true;
        };

        if (!ensureSceneExists(scene.nextSceneId, QStringLiteral("next"))) {
            return false;
        }

        if (!ensureSceneExists(scene.completionNextSceneId, QStringLiteral("completion_next"))) {
            return false;
        }

        for (const ConditionalSceneLink &variant : scene.nextVariants) {
            if (!ensureSceneExists(variant.nextSceneId, QStringLiteral("next_variants"))) {
                return false;
            }
        }

        for (const ConditionalSceneLink &variant : scene.completionNextVariants) {
            if (!ensureSceneExists(variant.nextSceneId, QStringLiteral("completion_next_variants"))) {
                return false;
            }
        }

        for (const NarrativeInteraction &interaction : scene.interactions) {
            if (!ensureSceneExists(interaction.nextSceneId, QStringLiteral("interaction next"))) {
                return false;
            }
        }
    }

    if (!result.startSceneId.isEmpty() && !result.scenes.contains(result.startSceneId)) {
        *errorMessage = QStringLiteral("start_scene references missing scene '%1'").arg(result.startSceneId);
        return false;
    }

    return true;
}

} // namespace

StoryLoadResult StoryLoader::loadFromFile(const QString &filePath)
{
    StoryLoadResult result;
    result.filePath = filePath;

    QFile file(filePath);
    if (!file.exists()) {
        result.errorMessage = QStringLiteral("Story file does not exist: %1").arg(filePath);
        return result;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        result.errorMessage = QStringLiteral("Failed to open story file: %1").arg(file.errorString());
        return result;
    }

    const QByteArray rawData = file.readAll();
    if (rawData.trimmed().isEmpty()) {
        result.errorMessage = QStringLiteral("Story file is empty");
        return result;
    }

    QJsonParseError parseError;
    const QJsonDocument document = QJsonDocument::fromJson(rawData, &parseError);
    if (parseError.error != QJsonParseError::NoError || !document.isObject()) {
        result.errorMessage = QStringLiteral("Failed to parse JSON: %1").arg(parseError.errorString());
        return result;
    }

    const QJsonObject rootObject = document.object();
    result.startSceneId = requireString(rootObject, QStringLiteral("start_scene"), &result.errorMessage);
    if (result.startSceneId.isEmpty() && !result.errorMessage.isEmpty()) {
        return result;
    }

    if (!rootObject.contains(QStringLiteral("scenes")) || !rootObject.value(QStringLiteral("scenes")).isArray()) {
        result.errorMessage = QStringLiteral("Missing or invalid scenes array");
        return result;
    }

    const QJsonArray scenesArray = rootObject.value(QStringLiteral("scenes")).toArray();
    if (scenesArray.isEmpty()) {
        result.errorMessage = QStringLiteral("Scenes array is empty");
        return result;
    }

    for (const QJsonValue &sceneValue : scenesArray) {
        if (!sceneValue.isObject()) {
            result.errorMessage = QStringLiteral("Each scene entry must be an object");
            return result;
        }

        const QJsonObject sceneObject = sceneValue.toObject();
        NarrativeScene scene;

        scene.id = requireString(sceneObject, QStringLiteral("id"), &result.errorMessage);
        if (scene.id.isEmpty() && !result.errorMessage.isEmpty()) {
            return result;
        }

        if (result.scenes.contains(scene.id)) {
            result.errorMessage = QStringLiteral("Duplicate scene id: %1").arg(scene.id);
            return result;
        }

        scene.header = sceneObject.value(QStringLiteral("header")).toString();
        scene.speaker = sceneObject.value(QStringLiteral("speaker")).toString();
        scene.text = sceneObject.value(QStringLiteral("text")).toString();
        scene.nextSceneId = sceneObject.value(QStringLiteral("next")).toString();
        scene.completionText = sceneObject.value(QStringLiteral("completion_text")).toString();
        scene.completionNextSceneId = sceneObject.value(QStringLiteral("completion_next")).toString();
        scene.requiresAllInteractions = sceneObject.value(QStringLiteral("requires_all_interactions")).toBool(false);
        if (!parseStringListWithFallback(sceneObject,
                                         QStringLiteral("progress_texts"),
                                         QStringLiteral("progress_text"),
                                         &scene.progressTexts,
                                         &result.errorMessage)) {
            return result;
        }

        const QString modeString = requireString(sceneObject, QStringLiteral("mode"), &result.errorMessage);
        if (modeString.isEmpty() && !result.errorMessage.isEmpty()) {
            return result;
        }

        bool ok = false;
        scene.displayMode = parseDisplayMode(modeString, &ok);
        if (!ok) {
            result.errorMessage = QStringLiteral("Unknown scene mode: %1").arg(modeString);
            return result;
        }

        const QString backgroundString = requireString(sceneObject, QStringLiteral("background"), &result.errorMessage);
        if (backgroundString.isEmpty() && !result.errorMessage.isEmpty()) {
            return result;
        }

        scene.backgroundStyle = parseBackgroundStyle(backgroundString, &ok);
        if (!ok) {
            result.errorMessage = QStringLiteral("Unknown background style: %1").arg(backgroundString);
            return result;
        }

        if (scene.displayMode == NarrativeDisplayMode::Interaction) {
            const QString interactionModeString = requireString(sceneObject, QStringLiteral("interaction_mode"), &result.errorMessage);
            if (interactionModeString.isEmpty() && !result.errorMessage.isEmpty()) {
                return result;
            }

            scene.interactionMode = parseInteractionMode(interactionModeString, &ok);
            if (!ok || scene.interactionMode == InteractionMode::None) {
                result.errorMessage = QStringLiteral("Unknown or invalid interaction_mode: %1").arg(interactionModeString);
                return result;
            }
        }

        if (sceneObject.contains(QStringLiteral("interactions"))) {
            const QJsonValue interactionsValue = sceneObject.value(QStringLiteral("interactions"));
            if (!interactionsValue.isArray()) {
                result.errorMessage = QStringLiteral("Field interactions must be an array");
                return result;
            }

            const QJsonArray interactionsArray = interactionsValue.toArray();
            for (const QJsonValue &interactionValue : interactionsArray) {
                if (!interactionValue.isObject()) {
                    result.errorMessage = QStringLiteral("Each interaction entry must be an object");
                    return result;
                }

                NarrativeInteraction interaction;
                if (!parseInteraction(interactionValue.toObject(), &interaction, &result.errorMessage)) {
                    return result;
                }

                scene.interactions.append(interaction);
                ++result.interactionCount;
            }
        }

        if (scene.displayMode == NarrativeDisplayMode::Interaction && scene.interactions.isEmpty()) {
            result.errorMessage = QStringLiteral("Interaction scene '%1' must define interactions").arg(scene.id);
            return result;
        }

        if (!parseBoolWrites(sceneObject, QStringLiteral("set_bool"), &scene.boolWrites, &result.errorMessage)) {
            return result;
        }

        if (!parseStringWrites(sceneObject, QStringLiteral("set_string"), &scene.stringWrites, &result.errorMessage)) {
            return result;
        }

        if (!parseConditionalTexts(sceneObject, QStringLiteral("text_variants"), &scene.textVariants, &result.errorMessage)) {
            return result;
        }

        if (!parseConditionalTexts(sceneObject, QStringLiteral("completion_text_variants"), &scene.completionTextVariants, &result.errorMessage)) {
            return result;
        }

        if (!parseConditionalSceneLinks(sceneObject, QStringLiteral("next_variants"), &scene.nextVariants, &result.errorMessage)) {
            return result;
        }

        if (!parseConditionalSceneLinks(sceneObject, QStringLiteral("completion_next_variants"), &scene.completionNextVariants, &result.errorMessage)) {
            return result;
        }

        if (!parseConditionalBackgrounds(sceneObject, QStringLiteral("background_variants"), &scene.backgroundVariants, &result.errorMessage)) {
            return result;
        }

        result.scenes.insert(scene.id, scene);
    }

    result.sceneCount = result.scenes.size();

    if (!validateReferences(result, &result.errorMessage)) {
        return result;
    }

    result.success = true;
    return result;
}

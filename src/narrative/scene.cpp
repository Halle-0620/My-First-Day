#include "scene.h"

#include "gamestate.h"

bool NarrativeCondition::isEmpty() const
{
    return !hasBoolCondition && !hasStringCondition;
}

bool NarrativeCondition::matches(const GameState &state) const
{
    if (hasBoolCondition) {
        if (!state.hasBool(boolKey) || state.boolValue(boolKey) != boolValue) {
            return false;
        }
    }

    if (hasStringCondition) {
        if (!state.hasString(stringKey) || state.stringValue(stringKey) != stringEquals) {
            return false;
        }
    }

    return true;
}

#include "gamestate.h"

void GameState::clear()
{
    m_boolValues.clear();
    m_stringValues.clear();
}

void GameState::setBool(const QString &key, bool value)
{
    m_boolValues.insert(key, value);
}

bool GameState::boolValue(const QString &key, bool defaultValue) const
{
    return m_boolValues.value(key, defaultValue);
}

bool GameState::hasBool(const QString &key) const
{
    return m_boolValues.contains(key);
}

void GameState::setString(const QString &key, const QString &value)
{
    m_stringValues.insert(key, value);
}

QString GameState::stringValue(const QString &key, const QString &defaultValue) const
{
    return m_stringValues.value(key, defaultValue);
}

bool GameState::hasString(const QString &key) const
{
    return m_stringValues.contains(key);
}

bool GameState::has(const QString &key) const
{
    return hasBool(key) || hasString(key);
}

#ifndef GAMESTATE_H
#define GAMESTATE_H

#include <QHash>
#include <QString>

class GameState
{
public:
    void clear();

    void setBool(const QString &key, bool value);
    bool boolValue(const QString &key, bool defaultValue = false) const;
    bool hasBool(const QString &key) const;

    void setString(const QString &key, const QString &value);
    QString stringValue(const QString &key, const QString &defaultValue = QString()) const;
    bool hasString(const QString &key) const;

    bool has(const QString &key) const;

private:
    QHash<QString, bool> m_boolValues;
    QHash<QString, QString> m_stringValues;
};

#endif // GAMESTATE_H

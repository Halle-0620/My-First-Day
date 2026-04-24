#ifndef CORE_INTERACTIONTYPES_H
#define CORE_INTERACTIONTYPES_H

#include <QList>
#include <QString>

enum class InteractionMode {
    None,
    Choice,
    Hotspot,
    Action,
    Topic,
    Object
};

enum class InteractionItemType {
    Choice,
    Hotspot,
    Action,
    Topic,
    Object
};

struct InteractionItem
{
    QString id;
    QString text;
    InteractionItemType type = InteractionItemType::Choice;
    bool enabled = true;
    bool visited = false;
    bool hidden = false;
};

using InteractionItems = QList<InteractionItem>;

#endif // CORE_INTERACTIONTYPES_H

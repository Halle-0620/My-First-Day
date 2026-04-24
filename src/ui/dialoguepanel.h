#ifndef DIALOGUEPANEL_H
#define DIALOGUEPANEL_H

#include <QHash>
#include <QWidget>

#include "interactiontypes.h"

class QLabel;
class QPushButton;
class QTextEdit;
class QGridLayout;

class DialoguePanel : public QWidget
{
    Q_OBJECT

public:
    explicit DialoguePanel(QWidget *parent = nullptr);

    void setSpeaker(const QString &speaker);
    void clearSpeaker();
    void setSpeakerVisible(bool visible);
    void setText(const QString &text);

    void setChoices(const QStringList &choices);
    void setChoicesVisible(bool visible);

    void setInteractionItems(const InteractionItems &items);
    void setInteractionMode(InteractionMode mode);
    void setInteractionVisible(bool visible);
    void clearInteractionState();
    void markInteractionItemVisited(const QString &id);
    void setInteractionItemEnabled(const QString &id, bool enabled);

    void setContinueVisible(bool visible);
    void setDialogueVisible(bool visible);
    bool isContinueVisible() const;
    bool hasVisibleInteractions() const;
    void focusFirstInteraction();

signals:
    void interactionTriggered(const QString &id);
    void continueRequested();

private:
    void clearInteractionButtons();
    void rebuildInteractionButtons();
    void refreshInteractionButtonStyle(QPushButton *button) const;
    int columnCountForMode() const;

    QLabel *m_nameLabel;
    QTextEdit *m_textBox;
    QWidget *m_interactionContainer;
    QGridLayout *m_interactionLayout;
    QPushButton *m_continueButton;
    InteractionMode m_interactionMode;
    InteractionItems m_interactionItems;
    QHash<QString, QPushButton *> m_interactionButtons;
};

#endif // DIALOGUEPANEL_H

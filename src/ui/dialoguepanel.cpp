#include "dialoguepanel.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStyle>
#include <QTextCursor>
#include <QTextEdit>
#include <QVBoxLayout>

namespace {

QString roleNameForItemType(InteractionItemType type)
{
    switch (type) {
    case InteractionItemType::Choice:
        return QStringLiteral("choice");
    case InteractionItemType::Hotspot:
        return QStringLiteral("hotspot");
    case InteractionItemType::Action:
        return QStringLiteral("action");
    case InteractionItemType::Topic:
        return QStringLiteral("topic");
    case InteractionItemType::Object:
        return QStringLiteral("object");
    }

    return QStringLiteral("choice");
}

} // namespace

DialoguePanel::DialoguePanel(QWidget *parent)
    : QWidget(parent),
      m_nameLabel(nullptr),
      m_textBox(nullptr),
      m_interactionContainer(nullptr),
      m_interactionLayout(nullptr),
      m_continueButton(nullptr),
      m_interactionMode(InteractionMode::None)
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setContentsMargins(0, 0, 0, 0);
    rootLayout->setSpacing(10);

    m_nameLabel = new QLabel(QStringLiteral("旁白"), this);
    m_nameLabel->setObjectName("nameLabel");
    m_nameLabel->setMinimumHeight(40);
    m_nameLabel->setAlignment(Qt::AlignCenter);
    rootLayout->addWidget(m_nameLabel, 0, Qt::AlignLeft);

    m_textBox = new QTextEdit(this);
    m_textBox->setObjectName("textBox");
    m_textBox->setReadOnly(true);
    m_textBox->setMinimumHeight(150);
    m_textBox->setFrameStyle(QFrame::NoFrame);
    rootLayout->addWidget(m_textBox);

    m_interactionContainer = new QWidget(this);
    m_interactionContainer->setObjectName("interactionContainer");
    m_interactionLayout = new QGridLayout(m_interactionContainer);
    m_interactionLayout->setContentsMargins(0, 0, 0, 0);
    m_interactionLayout->setHorizontalSpacing(10);
    m_interactionLayout->setVerticalSpacing(10);
    rootLayout->addWidget(m_interactionContainer);

    auto *footerLayout = new QHBoxLayout();
    footerLayout->addStretch();

    m_continueButton = new QPushButton(QStringLiteral("继续"), this);
    m_continueButton->setObjectName("continueButton");
    m_continueButton->setMinimumHeight(42);
    footerLayout->addWidget(m_continueButton, 0, Qt::AlignRight);
    rootLayout->addLayout(footerLayout);

    connect(m_continueButton, &QPushButton::clicked, this, &DialoguePanel::continueRequested);

    setStyleSheet(
        "DialoguePanel {"
        "    background-color: rgba(7, 10, 18, 170);"
        "    border: 1px solid rgba(255, 255, 255, 40);"
        "    border-radius: 20px;"
        "}"
        "QLabel#nameLabel {"
        "    background-color: rgba(232, 238, 255, 225);"
        "    color: rgb(23, 33, 58);"
        "    border-radius: 14px;"
        "    padding: 4px 18px;"
        "    font-size: 16px;"
        "    font-weight: 700;"
        "}"
        "QTextEdit#textBox {"
        "    background: transparent;"
        "    color: rgb(242, 245, 250);"
        "    selection-background-color: rgba(255, 255, 255, 50);"
        "    font-size: 17px;"
        "    line-height: 140%;"
        "    padding: 8px 10px;"
        "}"
        "QWidget#interactionContainer {"
        "    background: transparent;"
        "}"
        "QPushButton {"
        "    border: none;"
        "    border-radius: 12px;"
        "    padding: 10px 16px;"
        "    font-size: 15px;"
        "}"
        "QPushButton[interactionRole=\"choice\"] {"
        "    background-color: rgba(245, 248, 255, 220);"
        "    color: rgb(24, 33, 59);"
        "    min-height: 46px;"
        "}"
        "QPushButton[interactionRole=\"hotspot\"] {"
        "    background-color: rgba(255, 255, 255, 28);"
        "    color: rgb(242, 245, 250);"
        "    border: 1px solid rgba(255, 255, 255, 70);"
        "    min-height: 44px;"
        "}"
        "QPushButton[interactionRole=\"action\"] {"
        "    background-color: rgba(232, 238, 255, 210);"
        "    color: rgb(22, 30, 51);"
        "    min-height: 42px;"
        "    min-width: 130px;"
        "}"
        "QPushButton[interactionRole=\"topic\"] {"
        "    background-color: rgba(255, 255, 255, 20);"
        "    color: rgb(242, 245, 250);"
        "    border: 1px solid rgba(255, 255, 255, 65);"
        "    min-height: 44px;"
        "}"
        "QPushButton[interactionRole=\"object\"] {"
        "    background-color: rgba(255, 255, 255, 10);"
        "    color: rgb(230, 236, 252);"
        "    border: 1px solid rgba(255, 255, 255, 28);"
        "    min-height: 34px;"
        "    padding: 6px 12px;"
        "    border-radius: 999px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(255, 255, 255, 240);"
        "    color: rgb(24, 33, 59);"
        "}"
        "QPushButton:focus {"
        "    border: 2px solid rgba(120, 156, 255, 220);"
        "    padding: 8px 14px;"
        "}"
        "QPushButton[visited=\"true\"] {"
        "    background-color: rgba(152, 165, 194, 120);"
        "    color: rgba(240, 244, 255, 170);"
        "    border: 1px solid rgba(255, 255, 255, 24);"
        "}"
        "QPushButton:disabled {"
        "    color: rgba(240, 244, 255, 150);"
        "}"
        "QPushButton#continueButton {"
        "    background-color: rgba(245, 248, 255, 220);"
        "    color: rgb(24, 33, 59);"
        "    min-width: 110px;"
        "    font-weight: 700;"
        "}"
    );

    m_interactionContainer->hide();
}

void DialoguePanel::setSpeaker(const QString &speaker)
{
    m_nameLabel->setText(speaker);
    m_nameLabel->setVisible(!speaker.trimmed().isEmpty());
}

void DialoguePanel::clearSpeaker()
{
    m_nameLabel->clear();
    m_nameLabel->hide();
}

void DialoguePanel::setSpeakerVisible(bool visible)
{
    m_nameLabel->setVisible(visible && !m_nameLabel->text().trimmed().isEmpty());
}

void DialoguePanel::setText(const QString &text)
{
    m_textBox->setPlainText(text);
    m_textBox->moveCursor(QTextCursor::Start);
}

void DialoguePanel::setChoices(const QStringList &choices)
{
    InteractionItems items;
    items.reserve(choices.size());

    for (int index = 0; index < choices.size(); ++index) {
        items.append({
            QStringLiteral("choice_%1").arg(index),
            choices.at(index),
            InteractionItemType::Choice,
            true,
            false,
            false
        });
    }

    setInteractionMode(InteractionMode::Choice);
    setInteractionItems(items);
}

void DialoguePanel::setChoicesVisible(bool visible)
{
    if (m_interactionMode == InteractionMode::Choice) {
        setInteractionVisible(visible);
    } else if (!visible) {
        setInteractionVisible(false);
    }
}

void DialoguePanel::setInteractionItems(const InteractionItems &items)
{
    m_interactionItems = items;
    rebuildInteractionButtons();
}

void DialoguePanel::setInteractionMode(InteractionMode mode)
{
    m_interactionMode = mode;
    rebuildInteractionButtons();
}

void DialoguePanel::setInteractionVisible(bool visible)
{
    const bool hasVisibleButton = !m_interactionButtons.isEmpty();
    m_interactionContainer->setVisible(visible && hasVisibleButton);
}

void DialoguePanel::clearInteractionState()
{
    for (auto it = m_interactionButtons.begin(); it != m_interactionButtons.end(); ++it) {
        QPushButton *button = it.value();
        button->setEnabled(true);
        button->setProperty("visited", false);
        refreshInteractionButtonStyle(button);
    }
}

void DialoguePanel::markInteractionItemVisited(const QString &id)
{
    if (!m_interactionButtons.contains(id)) {
        return;
    }

    QPushButton *button = m_interactionButtons.value(id);
    button->setProperty("visited", true);
    button->setEnabled(false);
    refreshInteractionButtonStyle(button);
}

void DialoguePanel::setInteractionItemEnabled(const QString &id, bool enabled)
{
    if (!m_interactionButtons.contains(id)) {
        return;
    }

    QPushButton *button = m_interactionButtons.value(id);
    button->setEnabled(enabled);
    refreshInteractionButtonStyle(button);
}

void DialoguePanel::setContinueVisible(bool visible)
{
    m_continueButton->setVisible(visible);
}

void DialoguePanel::setDialogueVisible(bool visible)
{
    setVisible(visible);
}

bool DialoguePanel::isContinueVisible() const
{
    return m_continueButton->isVisible();
}

bool DialoguePanel::hasVisibleInteractions() const
{
    return m_interactionContainer->isVisible() && !m_interactionButtons.isEmpty();
}

void DialoguePanel::focusFirstInteraction()
{
    for (auto it = m_interactionButtons.cbegin(); it != m_interactionButtons.cend(); ++it) {
        if (it.value()->isEnabled() && it.value()->isVisible()) {
            it.value()->setFocus();
            return;
        }
    }
}

void DialoguePanel::clearInteractionButtons()
{
    m_interactionButtons.clear();

    while (QLayoutItem *item = m_interactionLayout->takeAt(0)) {
        if (item->widget()) {
            item->widget()->deleteLater();
        }
        delete item;
    }
}

void DialoguePanel::rebuildInteractionButtons()
{
    clearInteractionButtons();

    if (m_interactionMode == InteractionMode::None) {
        m_interactionContainer->hide();
        return;
    }

    const int columnCount = columnCountForMode();
    int visibleIndex = 0;

    for (const InteractionItem &item : m_interactionItems) {
        if (item.hidden) {
            continue;
        }

        auto *button = new QPushButton(item.text, m_interactionContainer);
        button->setProperty("interactionRole", roleNameForItemType(item.type));
        button->setProperty("visited", item.visited);
        button->setEnabled(item.enabled);

        connect(button, &QPushButton::clicked, this, [this, item]() {
            emit interactionTriggered(item.id);
        });

        const int row = visibleIndex / columnCount;
        const int column = visibleIndex % columnCount;
        m_interactionLayout->addWidget(button, row, column);
        m_interactionButtons.insert(item.id, button);
        refreshInteractionButtonStyle(button);

        ++visibleIndex;
    }

    const bool visible = !m_interactionButtons.isEmpty();
    m_interactionContainer->setVisible(visible);

    if (visible) {
        focusFirstInteraction();
    }
}

void DialoguePanel::refreshInteractionButtonStyle(QPushButton *button) const
{
    button->style()->unpolish(button);
    button->style()->polish(button);
    button->update();
}

int DialoguePanel::columnCountForMode() const
{
    switch (m_interactionMode) {
    case InteractionMode::Choice:
        return 1;
    case InteractionMode::Hotspot:
        return 2;
    case InteractionMode::Action:
        return 2;
    case InteractionMode::Topic:
        return 2;
    case InteractionMode::Object:
        return 3;
    case InteractionMode::None:
        return 1;
    }

    return 1;
}

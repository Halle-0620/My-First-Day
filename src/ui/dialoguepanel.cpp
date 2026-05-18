#include "dialoguepanel.h"

#include <QFrame>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QPainterPath>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QTextLayout>
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
      m_contentContainer(nullptr),
      m_contentLayout(nullptr),
      m_interactionContainer(nullptr),
      m_interactionLayout(nullptr),
      m_continueButton(nullptr),
      m_interactionMode(InteractionMode::None)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setAttribute(Qt::WA_StyledBackground, false);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    rootLayout->setContentsMargins(42, 40, 42, 22);
    rootLayout->setSpacing(0);

    m_nameLabel = new QLabel(QStringLiteral("旁白"), this);
    m_nameLabel->setObjectName("nameLabel");
    m_nameLabel->setFixedHeight(40);
    m_nameLabel->setMinimumWidth(148);
    m_nameLabel->setAlignment(Qt::AlignCenter);

    m_contentContainer = new QWidget(this);
    m_contentContainer->setObjectName("contentContainer");
    m_contentContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    m_contentLayout = new QVBoxLayout(m_contentContainer);
    m_contentLayout->setContentsMargins(4, 32, 4, 0);
    m_contentLayout->setSpacing(18);

    m_textBox = new QLabel(this);
    m_textBox->setObjectName("textBox");
    m_textBox->setWordWrap(true);
    m_textBox->setAlignment(Qt::AlignLeft | Qt::AlignTop);
    m_textBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::MinimumExpanding);
    m_textBox->setMinimumHeight(92);
    m_contentLayout->addWidget(m_textBox);

    m_contentLayout->addStretch(1);

    m_interactionContainer = new QWidget(this);
    m_interactionContainer->setObjectName("interactionContainer");
    m_interactionContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_interactionLayout = new QGridLayout(m_interactionContainer);
    m_interactionLayout->setContentsMargins(0, 4, 0, 2);
    m_interactionLayout->setHorizontalSpacing(22);
    m_interactionLayout->setVerticalSpacing(18);
    m_contentLayout->addWidget(m_interactionContainer);
    rootLayout->addWidget(m_contentContainer);

    auto *footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->addStretch();

    m_continueButton = new QPushButton(QString::fromUtf8(u8"▼"), this);
    m_continueButton->setObjectName("continueButton");
    m_continueButton->setFixedSize(42, 26);
    footerLayout->addWidget(m_continueButton, 0, Qt::AlignRight);
    rootLayout->addLayout(footerLayout);

    connect(m_continueButton, &QPushButton::clicked, this, [this]() {
        emit continueButtonClicked();
        emit continueRequested();
    });

    setStyleSheet(
        "QLabel#nameLabel {"
        "    background: rgba(215, 229, 248, 176);"
        "    color: rgb(22, 38, 70);"
        "    border: 1px solid rgba(234, 242, 255, 195);"
        "    border-radius: 11px;"
        "    padding: 2px 18px;"
        "    font-size: 15px;"
        "    font-weight: 700;"
        "}"
        "QLabel#textBox {"
        "    background: transparent;"
        "    color: rgb(238, 244, 255);"
        "    font-size: 23px;"
        "    line-height: 168%;"
        "    padding: 16px 12px 8px 12px;"
        "}"
        "QWidget#contentContainer {"
        "    background: transparent;"
        "}"
        "QWidget#interactionContainer {"
        "    background: transparent;"
        "}"
        "QPushButton {"
        "    border: 1px solid rgba(210, 225, 255, 130);"
        "    border-radius: 12px;"
        "    padding: 12px 18px;"
        "    font-size: 17px;"
        "}"
        "QPushButton[interactionRole=\"choice\"] {"
        "    background-color: rgba(10, 20, 40, 130);"
        "    color: rgb(238, 244, 255);"
        "    min-height: 58px;"
        "    padding: 12px 22px;"
        "}"
        "QPushButton[interactionRole=\"hotspot\"] {"
        "    background-color: rgba(10, 20, 40, 118);"
        "    color: rgb(238, 244, 255);"
        "    min-height: 44px;"
        "}"
        "QPushButton[interactionRole=\"action\"] {"
        "    background-color: rgba(10, 20, 40, 122);"
        "    color: rgb(238, 244, 255);"
        "    min-height: 44px;"
        "    min-width: 148px;"
        "    padding: 11px 24px;"
        "}"
        "QPushButton[interactionRole=\"topic\"] {"
        "    background-color: rgba(10, 20, 40, 118);"
        "    color: rgb(238, 244, 255);"
        "    min-height: 44px;"
        "}"
        "QPushButton[interactionRole=\"object\"] {"
        "    background-color: rgba(10, 20, 40, 112);"
        "    color: rgb(238, 244, 255);"
        "    min-height: 40px;"
        "    padding: 8px 16px;"
        "    border-radius: 999px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(30, 52, 88, 166);"
        "    color: rgb(245, 248, 255);"
        "    border: 1px solid rgba(226, 238, 255, 180);"
        "}"
        "QPushButton:focus {"
        "    border: 1px solid rgba(232, 242, 255, 210);"
        "}"
        "QPushButton[visited=\"true\"] {"
        "    background-color: rgba(25, 35, 54, 92);"
        "    color: rgba(210, 219, 237, 168);"
        "    border: 1px solid rgba(210, 225, 255, 86);"
        "}"
        "QPushButton:disabled {"
        "    background-color: rgba(25, 35, 54, 92);"
        "    color: rgba(210, 219, 237, 150);"
        "    border: 1px solid rgba(190, 198, 214, 72);"
        "}"
        "QPushButton#continueButton {"
        "    background-color: transparent;"
        "    color: rgba(238, 244, 255, 210);"
        "    border: none;"
        "    border-radius: 0px;"
        "    font-weight: 700;"
        "    font-size: 18px;"
        "    padding: 0px;"
        "}"
        "QPushButton#continueButton:hover {"
        "    background-color: transparent;"
        "    border: none;"
        "    color: rgba(255, 255, 255, 238);"
        "}"
    );

    m_interactionContainer->hide();
    updateNameplateGeometry();
}

void DialoguePanel::setSpeaker(const QString &speaker)
{
    m_nameLabel->setText(speaker);
    m_nameLabel->setVisible(!speaker.trimmed().isEmpty());
    updateNameplateGeometry();
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
    m_textBox->setText(text);
}

QStringList DialoguePanel::paginateTextFrames(const QString &text, int maxLinesPerFrame) const
{
    QString normalized = text;
    normalized.replace(QStringLiteral("\r\n"), QStringLiteral("\n"));

    if (normalized.trimmed().isEmpty()) {
        return {};
    }

    const int lineLimit = qMax(1, maxLinesPerFrame);
    const int availableWidth = qMax(120, m_textBox->contentsRect().width() - 4);

    QTextLayout layout(normalized, m_textBox->font());
    QTextOption option = layout.textOption();
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(option);

    QStringList visualLines;
    layout.beginLayout();
    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(availableWidth);
        QString lineText = normalized.mid(line.textStart(), line.textLength());
        lineText.remove(QChar('\n'));
        lineText = lineText.trimmed();
        if (!lineText.isEmpty()) {
            visualLines.append(lineText);
        }
    }
    layout.endLayout();

    if (visualLines.isEmpty()) {
        return {normalized.trimmed()};
    }

    QStringList frames;
    for (int index = 0; index < visualLines.size(); index += lineLimit) {
        frames.append(visualLines.mid(index, lineLimit).join(QStringLiteral("\n")));
    }

    return frames;
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

int DialoguePanel::visibleInteractionCount() const
{
    return m_interactionButtons.size();
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

void DialoguePanel::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    const QRectF panelRect = rect().adjusted(2, 16, -2, -2);

    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(4, 10, 20, 40));
    painter.drawRoundedRect(panelRect.translated(0, 7), 24, 24);

    QLinearGradient fill(panelRect.topLeft(), panelRect.bottomLeft());
    fill.setColorAt(0.0, QColor(8, 16, 32, 192));
    fill.setColorAt(0.55, QColor(8, 16, 32, 186));
    fill.setColorAt(1.0, QColor(7, 14, 28, 180));

    painter.setBrush(fill);
    painter.setPen(QPen(QColor(220, 235, 255, 170), 1.2));
    painter.drawRoundedRect(panelRect, 24, 24);

    painter.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(panelRect, 24, 24);
    painter.setClipPath(clipPath);
    QLinearGradient shine(panelRect.topLeft(), panelRect.bottomLeft());
    shine.setColorAt(0.0, QColor(255, 255, 255, 20));
    shine.setColorAt(0.22, QColor(210, 228, 255, 10));
    shine.setColorAt(1.0, QColor(255, 255, 255, 0));
    painter.fillRect(QRectF(panelRect.left(), panelRect.top(), panelRect.width(), panelRect.height() * 0.4), shine);
    painter.restore();
}

void DialoguePanel::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    updateNameplateGeometry();
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
    const bool isSingleActionButton = m_interactionMode == InteractionMode::Action
        && m_interactionItems.size() == 1;
    for (int column = 0; column < 4; ++column) {
        m_interactionLayout->setColumnStretch(column, column < columnCount ? 1 : 0);
    }
    int visibleIndex = 0;

    for (const InteractionItem &item : m_interactionItems) {
        if (item.hidden) {
            continue;
        }

        auto *button = new QPushButton(item.text, m_interactionContainer);
        button->setProperty("interactionRole", roleNameForItemType(item.type));
        button->setProperty("visited", item.visited);
        button->setEnabled(item.enabled);
        button->setSizePolicy(isSingleActionButton ? QSizePolicy::Maximum : QSizePolicy::Expanding,
                              QSizePolicy::Fixed);
        if (isSingleActionButton) {
            button->setMinimumWidth(196);
            button->setMaximumWidth(260);
        }

        connect(button, &QPushButton::clicked, this, [this, item]() {
            emit interactionButtonClicked(item.id);
            emit interactionTriggered(item.id);
        });

        const int row = visibleIndex / columnCount;
        const int column = visibleIndex % columnCount;
        m_interactionLayout->addWidget(button,
                                       row,
                                       column,
                                       isSingleActionButton ? Qt::AlignRight : Qt::Alignment());
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
        return m_interactionItems.size() <= 2 ? 2 : 2;
    case InteractionMode::Hotspot:
        return 2;
    case InteractionMode::Action:
        return m_interactionItems.size() <= 1 ? 1 : 2;
    case InteractionMode::Topic:
        return 2;
    case InteractionMode::Object:
        return 3;
    case InteractionMode::None:
        return 1;
    }

    return 1;
}

void DialoguePanel::updateNameplateGeometry()
{
    if (!m_nameLabel) {
        return;
    }

    const int preferredWidth = qBound(132,
                                      m_nameLabel->sizeHint().width() + 24,
                                      qMax(184, width() / 4));
    m_nameLabel->setFixedWidth(preferredWidth);
    m_nameLabel->move(46, 12);
    m_nameLabel->raise();
}

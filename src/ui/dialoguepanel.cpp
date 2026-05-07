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
      m_interactionContainer(nullptr),
      m_interactionLayout(nullptr),
      m_continueButton(nullptr),
      m_interactionMode(InteractionMode::None)
{
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setAttribute(Qt::WA_StyledBackground, false);

    auto *rootLayout = new QVBoxLayout(this);
    rootLayout->setSizeConstraint(QLayout::SetDefaultConstraint);
    rootLayout->setContentsMargins(30, 28, 30, 12);
    rootLayout->setSpacing(6);

    m_nameLabel = new QLabel(QStringLiteral("旁白"), this);
    m_nameLabel->setObjectName("nameLabel");
    m_nameLabel->setFixedHeight(34);
    m_nameLabel->setAlignment(Qt::AlignCenter);

    m_textBox = new QLabel(this);
    m_textBox->setObjectName("textBox");
    m_textBox->setWordWrap(true);
    m_textBox->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_textBox->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_textBox->setFixedHeight(fontMetrics().height() + 24);
    rootLayout->addWidget(m_textBox);

    m_interactionContainer = new QWidget(this);
    m_interactionContainer->setObjectName("interactionContainer");
    m_interactionContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Maximum);
    m_interactionLayout = new QGridLayout(m_interactionContainer);
    m_interactionLayout->setContentsMargins(0, 0, 0, 0);
    m_interactionLayout->setHorizontalSpacing(10);
    m_interactionLayout->setVerticalSpacing(10);
    rootLayout->addWidget(m_interactionContainer);

    auto *footerLayout = new QHBoxLayout();
    footerLayout->setContentsMargins(0, 0, 0, 0);
    footerLayout->addStretch();

    m_continueButton = new QPushButton(QStringLiteral(">"), this);
    m_continueButton->setObjectName("continueButton");
    m_continueButton->setFixedSize(44, 28);
    footerLayout->addWidget(m_continueButton, 0, Qt::AlignRight);
    rootLayout->addLayout(footerLayout);

    connect(m_continueButton, &QPushButton::clicked, this, [this]() {
        emit continueButtonClicked();
        emit continueRequested();
    });

    setStyleSheet(
        "QLabel#nameLabel {"
        "    background: qlineargradient(x1:0, y1:0, x2:1, y2:1,"
        "                                 stop:0 rgba(221, 232, 255, 240),"
        "                                 stop:1 rgba(170, 196, 236, 236));"
        "    color: rgb(58, 88, 145);"
        "    border: 2px solid rgba(255, 255, 255, 210);"
        "    border-radius: 15px;"
        "    padding: 2px 20px;"
        "    font-size: 15px;"
        "    font-weight: 700;"
        "}"
        "QLabel#textBox {"
        "    background: transparent;"
        "    color: rgb(223, 232, 250);"
        "    font-size: 16px;"
        "    line-height: 150%;"
        "    padding: 4px 10px 2px 10px;"
        "}"
        "QWidget#interactionContainer {"
        "    background: transparent;"
        "}"
        "QPushButton {"
        "    border: 1px solid rgba(255, 255, 255, 165);"
        "    border-radius: 14px;"
        "    padding: 8px 14px;"
        "    font-size: 14px;"
        "}"
        "QPushButton[interactionRole=\"choice\"] {"
        "    background-color: rgba(53, 79, 132, 224);"
        "    color: rgb(232, 239, 252);"
        "    min-height: 38px;"
        "}"
        "QPushButton[interactionRole=\"hotspot\"] {"
        "    background-color: rgba(40, 63, 108, 198);"
        "    color: rgb(224, 233, 250);"
        "    min-height: 38px;"
        "}"
        "QPushButton[interactionRole=\"action\"] {"
        "    background-color: rgba(71, 95, 149, 220);"
        "    color: rgb(230, 238, 252);"
        "    min-height: 36px;"
        "    min-width: 130px;"
        "}"
        "QPushButton[interactionRole=\"topic\"] {"
        "    background-color: rgba(40, 63, 108, 198);"
        "    color: rgb(224, 233, 250);"
        "    min-height: 38px;"
        "}"
        "QPushButton[interactionRole=\"object\"] {"
        "    background-color: rgba(28, 46, 83, 188);"
        "    color: rgb(221, 232, 250);"
        "    min-height: 34px;"
        "    padding: 6px 12px;"
        "    border-radius: 999px;"
        "}"
        "QPushButton:hover {"
        "    background-color: rgba(82, 113, 176, 236);"
        "    color: rgb(245, 248, 255);"
        "}"
        "QPushButton:focus {"
        "    border: 2px solid rgba(149, 188, 255, 220);"
        "    padding: 9px 17px;"
        "}"
        "QPushButton[visited=\"true\"] {"
        "    background-color: rgba(93, 111, 145, 155);"
        "    color: rgba(210, 219, 237, 178);"
        "    border: 1px solid rgba(255, 255, 255, 96);"
        "}"
        "QPushButton:disabled {"
        "    color: rgba(210, 219, 237, 150);"
        "}"
        "QPushButton#continueButton {"
        "    background-color: rgba(227, 235, 251, 228);"
        "    color: rgb(69, 96, 148);"
        "    border-radius: 14px;"
        "    font-weight: 700;"
        "    font-size: 15px;"
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
    painter.setBrush(QColor(9, 18, 36, 58));
    painter.drawRoundedRect(panelRect.translated(0, 7), 24, 24);

    QLinearGradient fill(panelRect.topLeft(), panelRect.bottomLeft());
    fill.setColorAt(0.0, QColor(29, 43, 78, 230));
    fill.setColorAt(0.58, QColor(19, 31, 58, 220));
    fill.setColorAt(1.0, QColor(13, 22, 42, 212));

    painter.setBrush(fill);
    painter.setPen(QPen(QColor(255, 255, 255, 214), 3.0));
    painter.drawRoundedRect(panelRect, 24, 24);

    painter.save();
    QPainterPath clipPath;
    clipPath.addRoundedRect(panelRect, 24, 24);
    painter.setClipPath(clipPath);
    painter.setPen(Qt::NoPen);
    painter.setBrush(QColor(152, 188, 255, 24));

    const qreal dotSpacing = 36.0;
    const qreal dotRadius = 7.0;
    for (qreal y = panelRect.top() + 16.0; y < panelRect.bottom() + dotSpacing; y += dotSpacing) {
        for (qreal x = panelRect.left() + 18.0; x < panelRect.right() + dotSpacing; x += dotSpacing) {
            painter.drawEllipse(QPointF(x, y), dotRadius, dotRadius);
        }
    }

    QLinearGradient shine(panelRect.topLeft(), panelRect.topRight());
    shine.setColorAt(0.0, QColor(255, 255, 255, 65));
    shine.setColorAt(0.45, QColor(255, 255, 255, 15));
    shine.setColorAt(1.0, QColor(197, 219, 255, 42));
    painter.fillRect(QRectF(panelRect.left(), panelRect.top(), panelRect.width(), panelRect.height() * 0.33), shine);
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
            emit interactionButtonClicked(item.id);
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
        return 2;
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

void DialoguePanel::updateNameplateGeometry()
{
    if (!m_nameLabel) {
        return;
    }

    const int preferredWidth = qBound(170, width() / 6, 230);
    m_nameLabel->setFixedWidth(preferredWidth);
    m_nameLabel->move(54, 0);
    m_nameLabel->raise();
}

#include "floatingchoicelayer.h"

#include <QFontMetrics>
#include <QGraphicsDropShadowEffect>
#include <QPushButton>
#include <QResizeEvent>
#include <QStyle>
#include <QTextLayout>

namespace {

QString wrapVisualLines(const QString &text, const QFont &font, int maxWidth)
{
    const QString normalized = text.trimmed();
    if (normalized.isEmpty()) {
        return {};
    }

    QTextLayout layout(normalized, font);
    QTextOption option;
    option.setWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    layout.setTextOption(option);

    QStringList lines;
    layout.beginLayout();
    while (true) {
        QTextLine line = layout.createLine();
        if (!line.isValid()) {
            break;
        }

        line.setLineWidth(maxWidth);
        QString lineText = normalized.mid(line.textStart(), line.textLength());
        lineText.remove(QChar('\n'));
        lines.append(lineText.trimmed());
    }
    layout.endLayout();

    return lines.join(QStringLiteral("\n"));
}

QGraphicsDropShadowEffect *createGlowEffect(QObject *parent)
{
    auto *effect = new QGraphicsDropShadowEffect(parent);
    effect->setBlurRadius(28);
    effect->setOffset(0, 8);
    effect->setColor(QColor(12, 10, 4, 120));
    return effect;
}

} // namespace

FloatingChoiceLayer::FloatingChoiceLayer(QWidget *parent)
    : QWidget(parent),
      m_leftButton(new QPushButton(this)),
      m_rightButton(new QPushButton(this)),
      m_leftAvoidance(0)
{
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(QStringLiteral("background: transparent;"));

    const QString buttonStyle =
        "QPushButton {"
        "    background: transparent;"
        "    color: rgba(249, 242, 222, 228);"
        "    border: none;"
        "    padding: 18px 22px;"
        "    font-size: 32px;"
        "    font-weight: 600;"
        "    text-align: center;"
        "}"
        "QPushButton:hover {"
        "    color: rgba(255, 248, 230, 255);"
        "}"
        "QPushButton:pressed {"
        "    color: rgba(235, 224, 199, 235);"
        "    padding-top: 20px;"
        "    padding-bottom: 16px;"
        "}";

    for (QPushButton *button : {m_leftButton, m_rightButton}) {
        button->setCursor(Qt::PointingHandCursor);
        button->setStyleSheet(buttonStyle);
        button->setFlat(true);
        button->setGraphicsEffect(createGlowEffect(button));
        button->hide();
    }

    connect(m_leftButton, &QPushButton::clicked, this, [this]() {
        if (!m_choiceIds.isEmpty()) {
            emit choiceButtonClicked(m_choiceIds.at(0));
            emit choiceTriggered(m_choiceIds.at(0));
        }
    });
    connect(m_rightButton, &QPushButton::clicked, this, [this]() {
        if (m_choiceIds.size() > 1) {
            emit choiceButtonClicked(m_choiceIds.at(1));
            emit choiceTriggered(m_choiceIds.at(1));
        }
    });
}

void FloatingChoiceLayer::setChoices(const InteractionItems &items)
{
    m_choiceIds.clear();
    m_choiceTexts.clear();

    for (const InteractionItem &item : items) {
        if (item.hidden) {
            continue;
        }

        m_choiceIds.append(item.id);
        m_choiceTexts.append(item.text);
        if (m_choiceIds.size() == 2) {
            break;
        }
    }

    updateButton(m_leftButton, 0);
    updateButton(m_rightButton, 1);
    refreshLayout();
}

void FloatingChoiceLayer::clearChoices()
{
    m_choiceIds.clear();
    m_choiceTexts.clear();
    m_leftButton->hide();
    m_rightButton->hide();
}

bool FloatingChoiceLayer::hasChoices() const
{
    return !m_choiceIds.isEmpty();
}

int FloatingChoiceLayer::visibleChoiceCount() const
{
    return m_choiceIds.size();
}

void FloatingChoiceLayer::setLeftAvoidance(int pixels)
{
    m_leftAvoidance = qMax(0, pixels);
    refreshLayout();
}

void FloatingChoiceLayer::refreshLayout()
{
    if (!hasChoices() || width() <= 0 || height() <= 0) {
        return;
    }

    if (m_choiceIds.size() == 1) {
        const int maxButtonWidth = qBound(280, qRound(width() * 0.34), 460);
        updateButton(m_leftButton, 0);
        m_rightButton->hide();

        const QSize centerSize = wrappedButtonSize(m_choiceTexts.at(0), maxButtonWidth);
        const int centerX = width() / 2;
        const int desiredCenterY = qRound(height() * 0.61);
        const int bottomSafeMargin = qMax(138, qRound(height() * 0.18));
        const int maxCenterY = height() - bottomSafeMargin - centerSize.height() / 2;
        const int yCenter = qMin(desiredCenterY, maxCenterY);

        m_leftButton->setGeometry(centerX - centerSize.width() / 2,
                                  yCenter - centerSize.height() / 2,
                                  centerSize.width(),
                                  centerSize.height());
        m_leftButton->raise();
        return;
    }

    const int sidePadding = qMax(28, qRound(width() * 0.04));
    const int centerGap = qMax(88, qRound(width() * 0.10));
    const int leftAnchor = qRound(width() * 0.27);
    const int rightAnchor = qRound(width() * 0.73);
    const int maxLeftWidth = qMax(220, rightAnchor - centerGap / 2 - qMax(leftAnchor, m_leftAvoidance) - 12);
    const int maxRightWidth = qMax(220, width() - sidePadding - (rightAnchor + centerGap / 2));
    const int maxButtonWidth = qMin(360, qMin(maxLeftWidth, maxRightWidth));
    updateButton(m_leftButton, 0);
    updateButton(m_rightButton, 1);

    const int leftCenter = qMax(leftAnchor, m_leftAvoidance + maxButtonWidth / 2);
    const int rightCenter = qRound(width() * 0.73);

    const QSize leftSize = wrappedButtonSize(m_choiceTexts.at(0), maxButtonWidth);
    const QSize rightSize = wrappedButtonSize(m_choiceTexts.at(1), maxButtonWidth);
    const int bottomSafeMargin = qMax(150, qRound(height() * 0.23));
    const int desiredCenterY = qRound(height() * 0.49);
    const int maxCenterY = height() - bottomSafeMargin - qMax(leftSize.height(), rightSize.height()) / 2;
    const int yCenter = qMin(desiredCenterY, maxCenterY);

    m_leftButton->setGeometry(leftCenter - leftSize.width() / 2,
                              yCenter - leftSize.height() / 2,
                              leftSize.width(),
                              leftSize.height());
    m_rightButton->setGeometry(rightCenter - rightSize.width() / 2,
                               yCenter - rightSize.height() / 2,
                               rightSize.width(),
                               rightSize.height());
}

void FloatingChoiceLayer::resizeEvent(QResizeEvent *event)
{
    QWidget::resizeEvent(event);
    refreshLayout();
}

void FloatingChoiceLayer::updateButton(QPushButton *button, int index)
{
    if (index >= m_choiceIds.size() || index >= m_choiceTexts.size()) {
        button->hide();
        return;
    }

    const int maxButtonWidth = m_choiceIds.size() == 1
        ? qBound(280, qRound(width() * 0.34), 460)
        : qBound(280, qRound(width() * 0.24), 360);
    const QSize size = wrappedButtonSize(m_choiceTexts.at(index), maxButtonWidth);
    button->setText(wrappedButtonText(m_choiceTexts.at(index), maxButtonWidth));
    button->setFixedSize(size);
    button->show();
    button->raise();
}

QSize FloatingChoiceLayer::wrappedButtonSize(const QString &text, int maxWidth) const
{
    const QString wrapped = wrappedButtonText(text, maxWidth);
    const QFontMetrics metrics(m_leftButton->font());
    const QRect textRect = metrics.boundingRect(QRect(0, 0, maxWidth, 200),
                                                Qt::TextWordWrap | Qt::AlignCenter,
                                                wrapped);
    return QSize(qBound(220, textRect.width() + 56, maxWidth + 44),
                 qMax(74, textRect.height() + 36));
}

QString FloatingChoiceLayer::wrappedButtonText(const QString &text, int maxWidth) const
{
    return wrapVisualLines(text, m_leftButton->font(), qMax(180, maxWidth - 44));
}

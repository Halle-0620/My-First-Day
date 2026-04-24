#include "backgroundwidget.h"

#include <QLinearGradient>
#include <QPainter>

BackgroundWidget::BackgroundWidget(QWidget *parent)
    : QWidget(parent),
      m_overlayTint(12, 18, 32, 90),
      m_backgroundStyle(BackgroundStyle::ClassroomDusk)
{
    setAutoFillBackground(false);
}

void BackgroundWidget::setBackgroundPixmap(const QPixmap &pixmap)
{
    m_backgroundPixmap = pixmap;
    update();
}

void BackgroundWidget::clearBackgroundPixmap()
{
    m_backgroundPixmap = QPixmap();
    update();
}

void BackgroundWidget::setOverlayTint(const QColor &color)
{
    m_overlayTint = color;
    update();
}

void BackgroundWidget::setBackgroundStyle(BackgroundStyle style)
{
    m_backgroundStyle = style;

    switch (m_backgroundStyle) {
    case BackgroundStyle::ClassroomDusk:
        m_overlayTint = QColor(12, 18, 32, 90);
        break;
    case BackgroundStyle::QuietHallway:
        m_overlayTint = QColor(9, 18, 34, 96);
        break;
    case BackgroundStyle::ChoiceFocus:
        m_overlayTint = QColor(20, 18, 28, 84);
        break;
    case BackgroundStyle::SoftNarration:
        m_overlayTint = QColor(18, 20, 32, 72);
        break;
    case BackgroundStyle::NightRain:
        m_overlayTint = QColor(6, 10, 24, 112);
        break;
    case BackgroundStyle::EndingGlow:
        m_overlayTint = QColor(22, 18, 28, 78);
        break;
    case BackgroundStyle::EndingBlack:
        m_overlayTint = QColor(0, 0, 0, 132);
        break;
    case BackgroundStyle::DreamDrift:
        m_overlayTint = QColor(18, 10, 34, 120);
        break;
    }

    update();
}

BackgroundWidget::BackgroundStyle BackgroundWidget::backgroundStyle() const
{
    return m_backgroundStyle;
}

void BackgroundWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing, true);

    if (!m_backgroundPixmap.isNull()) {
        painter.drawPixmap(rect(), m_backgroundPixmap);
    } else {
        QLinearGradient gradient(rect().topLeft(), rect().bottomLeft());

        switch (m_backgroundStyle) {
        case BackgroundStyle::ClassroomDusk:
            gradient.setColorAt(0.0, QColor(42, 58, 91));
            gradient.setColorAt(0.55, QColor(73, 93, 136));
            gradient.setColorAt(1.0, QColor(19, 25, 42));
            break;
        case BackgroundStyle::QuietHallway:
            gradient.setColorAt(0.0, QColor(25, 43, 77));
            gradient.setColorAt(0.55, QColor(46, 74, 122));
            gradient.setColorAt(1.0, QColor(12, 20, 34));
            break;
        case BackgroundStyle::ChoiceFocus:
            gradient.setColorAt(0.0, QColor(66, 74, 102));
            gradient.setColorAt(0.5, QColor(95, 87, 112));
            gradient.setColorAt(1.0, QColor(24, 24, 36));
            break;
        case BackgroundStyle::SoftNarration:
            gradient.setColorAt(0.0, QColor(54, 63, 89));
            gradient.setColorAt(0.58, QColor(86, 92, 121));
            gradient.setColorAt(1.0, QColor(26, 29, 43));
            break;
        case BackgroundStyle::NightRain:
            gradient.setColorAt(0.0, QColor(16, 30, 56));
            gradient.setColorAt(0.56, QColor(20, 43, 79));
            gradient.setColorAt(1.0, QColor(6, 12, 24));
            break;
        case BackgroundStyle::EndingGlow:
            gradient.setColorAt(0.0, QColor(84, 72, 106));
            gradient.setColorAt(0.55, QColor(118, 96, 116));
            gradient.setColorAt(1.0, QColor(37, 28, 43));
            break;
        case BackgroundStyle::EndingBlack:
            gradient.setColorAt(0.0, QColor(18, 18, 22));
            gradient.setColorAt(0.5, QColor(9, 10, 14));
            gradient.setColorAt(1.0, QColor(0, 0, 0));
            break;
        case BackgroundStyle::DreamDrift:
            gradient.setColorAt(0.0, QColor(49, 38, 78));
            gradient.setColorAt(0.5, QColor(23, 27, 58));
            gradient.setColorAt(1.0, QColor(7, 8, 18));
            break;
        }

        painter.fillRect(rect(), gradient);
    }

    painter.fillRect(rect(), m_overlayTint);

    QLinearGradient vignette(rect().topLeft(), rect().bottomRight());
    vignette.setColorAt(0.0, QColor(255, 255, 255, 24));
    vignette.setColorAt(0.45, QColor(255, 255, 255, 0));
    vignette.setColorAt(1.0, QColor(0, 0, 0, 80));
    painter.fillRect(rect(), vignette);
}

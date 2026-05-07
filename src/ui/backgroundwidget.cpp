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
    case BackgroundStyle::DeskDusk:
        m_overlayTint = QColor(20, 22, 30, 42);
        break;
    case BackgroundStyle::DeskNight:
        m_overlayTint = QColor(10, 14, 22, 72);
        break;
    case BackgroundStyle::Hallway:
        m_overlayTint = QColor(8, 16, 28, 78);
        break;
    case BackgroundStyle::TreeUnder:
        m_overlayTint = QColor(10, 16, 20, 68);
        break;
    case BackgroundStyle::GymBack:
        m_overlayTint = QColor(10, 14, 26, 76);
        break;
    case BackgroundStyle::Office:
        m_overlayTint = QColor(18, 18, 24, 40);
        break;
    case BackgroundStyle::Toilet:
        m_overlayTint = QColor(20, 24, 28, 46);
        break;
    case BackgroundStyle::Dismissal:
        m_overlayTint = QColor(16, 18, 24, 54);
        break;
    case BackgroundStyle::SchoolGate:
        m_overlayTint = QColor(12, 18, 26, 62);
        break;
    case BackgroundStyle::Ebike:
        m_overlayTint = QColor(8, 12, 24, 86);
        break;
    case BackgroundStyle::Home:
        m_overlayTint = QColor(18, 16, 16, 34);
        break;
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
    case BackgroundStyle::Message1:
    case BackgroundStyle::Message2:
    case BackgroundStyle::Message3:
        m_overlayTint = QColor(18, 16, 16, 18);
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
        case BackgroundStyle::DeskDusk:
            gradient.setColorAt(0.0, QColor(76, 88, 112));
            gradient.setColorAt(0.55, QColor(112, 122, 148));
            gradient.setColorAt(1.0, QColor(44, 48, 60));
            break;
        case BackgroundStyle::DeskNight:
            gradient.setColorAt(0.0, QColor(38, 52, 82));
            gradient.setColorAt(0.55, QColor(56, 72, 104));
            gradient.setColorAt(1.0, QColor(18, 24, 38));
            break;
        case BackgroundStyle::Hallway:
            gradient.setColorAt(0.0, QColor(34, 52, 86));
            gradient.setColorAt(0.55, QColor(54, 78, 118));
            gradient.setColorAt(1.0, QColor(16, 24, 38));
            break;
        case BackgroundStyle::TreeUnder:
            gradient.setColorAt(0.0, QColor(42, 58, 54));
            gradient.setColorAt(0.55, QColor(62, 88, 78));
            gradient.setColorAt(1.0, QColor(18, 28, 24));
            break;
        case BackgroundStyle::GymBack:
            gradient.setColorAt(0.0, QColor(34, 42, 72));
            gradient.setColorAt(0.55, QColor(52, 64, 96));
            gradient.setColorAt(1.0, QColor(16, 20, 34));
            break;
        case BackgroundStyle::Office:
            gradient.setColorAt(0.0, QColor(116, 124, 138));
            gradient.setColorAt(0.55, QColor(146, 152, 164));
            gradient.setColorAt(1.0, QColor(70, 74, 84));
            break;
        case BackgroundStyle::Toilet:
            gradient.setColorAt(0.0, QColor(124, 132, 140));
            gradient.setColorAt(0.55, QColor(156, 162, 170));
            gradient.setColorAt(1.0, QColor(76, 80, 88));
            break;
        case BackgroundStyle::Dismissal:
            gradient.setColorAt(0.0, QColor(82, 94, 112));
            gradient.setColorAt(0.55, QColor(110, 120, 136));
            gradient.setColorAt(1.0, QColor(42, 46, 56));
            break;
        case BackgroundStyle::SchoolGate:
            gradient.setColorAt(0.0, QColor(40, 56, 88));
            gradient.setColorAt(0.55, QColor(56, 78, 118));
            gradient.setColorAt(1.0, QColor(18, 28, 42));
            break;
        case BackgroundStyle::Ebike:
            gradient.setColorAt(0.0, QColor(18, 32, 58));
            gradient.setColorAt(0.55, QColor(24, 46, 82));
            gradient.setColorAt(1.0, QColor(8, 14, 28));
            break;
        case BackgroundStyle::Home:
            gradient.setColorAt(0.0, QColor(112, 98, 84));
            gradient.setColorAt(0.55, QColor(144, 126, 110));
            gradient.setColorAt(1.0, QColor(58, 48, 42));
            break;
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
        case BackgroundStyle::Message1:
        case BackgroundStyle::Message2:
        case BackgroundStyle::Message3:
            gradient.setColorAt(0.0, QColor(112, 98, 84));
            gradient.setColorAt(0.55, QColor(144, 126, 110));
            gradient.setColorAt(1.0, QColor(58, 48, 42));
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

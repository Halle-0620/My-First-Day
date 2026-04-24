#ifndef BACKGROUNDWIDGET_H
#define BACKGROUNDWIDGET_H

#include "../core/backgroundstyle.h"

#include <QColor>
#include <QPixmap>
#include <QWidget>

class BackgroundWidget : public QWidget
{
    Q_OBJECT

public:
    using BackgroundStyle = ::BackgroundStyle;

    explicit BackgroundWidget(QWidget *parent = nullptr);

    void setBackgroundPixmap(const QPixmap &pixmap);
    void clearBackgroundPixmap();
    void setOverlayTint(const QColor &color);
    void setBackgroundStyle(BackgroundStyle style);
    BackgroundStyle backgroundStyle() const;

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    QPixmap m_backgroundPixmap;
    QColor m_overlayTint;
    BackgroundStyle m_backgroundStyle;
};

#endif // BACKGROUNDWIDGET_H

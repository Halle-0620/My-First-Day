#ifndef FLOATINGCHOICELAYER_H
#define FLOATINGCHOICELAYER_H

#include "interactiontypes.h"

#include <QWidget>

class QPushButton;

class FloatingChoiceLayer : public QWidget
{
    Q_OBJECT

public:
    explicit FloatingChoiceLayer(QWidget *parent = nullptr);

    void setChoices(const InteractionItems &items);
    void clearChoices();
    bool hasChoices() const;
    int visibleChoiceCount() const;
    void setLeftAvoidance(int pixels);
    void refreshLayout();

signals:
    void choiceTriggered(const QString &id);
    void choiceButtonClicked(const QString &id);

protected:
    void resizeEvent(QResizeEvent *event) override;

private:
    void updateButton(QPushButton *button, int index);
    QSize wrappedButtonSize(const QString &text, int maxWidth) const;
    QString wrappedButtonText(const QString &text, int maxWidth) const;

    QPushButton *m_leftButton;
    QPushButton *m_rightButton;
    QStringList m_choiceIds;
    QStringList m_choiceTexts;
    int m_leftAvoidance;
};

#endif // FLOATINGCHOICELAYER_H

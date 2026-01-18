#ifndef STATEWIDGET_H
#define STATEWIDGET_H

#include <QToolButton>
#include <QLabel>
#include "global.h"

class QEnterEvent;
class QMouseEvent;
class QPaintEvent;

class StateWidget : public QToolButton
{
    Q_OBJECT
public:
    explicit StateWidget(QWidget *parent = nullptr);

    void SetState(QString normal="", QString hover="", QString press="",
                  QString select="", QString select_hover="", QString select_press="");

    ClickLbState GetCurState();
    void ClearState();

    void SetSelected(bool bselected);
    void AddRedPoint();
    void ShowRedPoint(bool show=true);

protected:
    void paintEvent(QPaintEvent* event) override;

    void mousePressEvent(QMouseEvent *ev) override;
    void mouseReleaseEvent(QMouseEvent *ev) override;
    void enterEvent(QEnterEvent* event) override;
    void leaveEvent(QEvent* event) override;

private:
    void ApplyState(const QString &state);

    QString _normal;
    QString _normal_hover;
    QString _normal_press;

    QString _selected;
    QString _selected_hover;
    QString _selected_press;

    ClickLbState _curstate = ClickLbState::Normal;
    QLabel * _red_point = nullptr;
};
#endif // STATEWIDGET_H

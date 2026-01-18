#include "statewidget.h"

#include <QEnterEvent>
#include <QMouseEvent>
#include <QVBoxLayout>

StateWidget::StateWidget(QWidget *parent)
    : QToolButton(parent),
      _curstate(ClickLbState::Normal)
{
    setCursor(Qt::PointingHandCursor);
    setFocusPolicy(Qt::NoFocus);
    AddRedPoint();
}

void StateWidget::paintEvent(QPaintEvent *event)
{
    QToolButton::paintEvent(event);
}

void StateWidget::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (_curstate == ClickLbState::Normal) {
            _curstate = ClickLbState::Selected;
            ApplyState(_selected_press);
        }
    }
    QToolButton::mousePressEvent(event);
}

void StateWidget::mouseReleaseEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        if (_curstate == ClickLbState::Normal) {
            ApplyState(_normal_hover);
        } else {
            ApplyState(_selected_hover);
        }
    }
    QToolButton::mouseReleaseEvent(event);
}

void StateWidget::enterEvent(QEnterEvent *event)
{
    if (_curstate == ClickLbState::Normal) {
        ApplyState(_normal_hover);
    } else {
        ApplyState(_selected_hover);
    }
    QToolButton::enterEvent(event);
}

void StateWidget::leaveEvent(QEvent *event)
{
    if (_curstate == ClickLbState::Normal) {
        ApplyState(_normal);
    } else {
        ApplyState(_selected);
    }
    QToolButton::leaveEvent(event);
}

void StateWidget::SetState(QString normal, QString hover, QString press,
                           QString select, QString select_hover, QString select_press)
{
    _normal = normal;
    _normal_hover = hover;
    _normal_press = press;

    _selected = select.isEmpty() ? normal : select;
    _selected_hover = select_hover.isEmpty() ? hover : select_hover;
    _selected_press = select_press.isEmpty() ? press : select_press;

    ApplyState(_normal);
}

ClickLbState StateWidget::GetCurState()
{
    return _curstate;
}

void StateWidget::ClearState()
{
    _curstate = ClickLbState::Normal;
    ApplyState(_normal);
}

void StateWidget::SetSelected(bool bselected)
{
    if (bselected) {
        _curstate = ClickLbState::Selected;
        ApplyState(_selected);
        return;
    }

    _curstate = ClickLbState::Normal;
    ApplyState(_normal);
}

void StateWidget::AddRedPoint()
{
    if (_red_point) {
        return;
    }

    _red_point = new QLabel(this);
    _red_point->setObjectName("red_point");
    _red_point->setAlignment(Qt::AlignCenter);

    auto *layout = new QVBoxLayout(this);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);
    layout->addWidget(_red_point, 0, Qt::AlignCenter);

    _red_point->setVisible(false);
}

void StateWidget::ShowRedPoint(bool show)
{
    if (!_red_point) {
        AddRedPoint();
    }
    _red_point->setVisible(show);
}

void StateWidget::ApplyState(const QString &state)
{
    setProperty("state", state);
    repolish(this);
    update();
}

#include "bubbleframe.h"

#include <QPainter>
#include <QDebug>
BubbleFrame::BubbleFrame(ChatRole role, QWidget *parent)
    :QFrame(parent)
    ,m_role(role)
    ,m_margin(3)
{
    m_pHLayout = new QHBoxLayout();
    m_pHLayout->setContentsMargins(6, 6, 6, 6);
    m_pHLayout->setSpacing(0);
    this->setLayout(m_pHLayout);
}

void BubbleFrame::setMargin(int margin)
{
    Q_UNUSED(margin);
}

void BubbleFrame::setWidget(QWidget *w)
{
    if(m_pHLayout->count() > 0)
        return ;
    else{
        m_pHLayout->addWidget(w);
    }
}

void BubbleFrame::paintEvent(QPaintEvent *e)
{
    QPainter painter(this);
    painter.setPen(Qt::NoPen);

    if(m_role == ChatRole::Other)
    {
        //画气泡
        QColor bk_color(Qt::white);
        painter.setBrush(QBrush(bk_color));
        QRect bk_rect = QRect(0, 0, this->width(), this->height());
        painter.drawRoundedRect(bk_rect,11,11);
    }
    else
    {
        QColor bk_color("#CCEBFF");
        painter.setBrush(QBrush(bk_color));
        //画气泡
        QRect bk_rect = QRect(0, 0, this->width(), this->height());
        painter.drawRoundedRect(bk_rect,12,12);
    }

    return QFrame::paintEvent(e);
}

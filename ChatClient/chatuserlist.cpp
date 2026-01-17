#include "ChatUserList.h"
#include "shortscrollbar.h"
#include <QFile>
#include <QScrollBar>
#include <QWheelEvent>
#include <QDebug>
#include <QStyle>
#include <QApplication>
#include <QCursor>
#include <QMouseEvent>

ChatUserList::ChatUserList(QWidget *parent)
    : QListWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    // 用自定义短滚动条
    auto *sb = new ShortScrollBar(Qt::Vertical, this);
    setVerticalScrollBar(sb);

    // qss（qproperty 版本）
    QFile f(":/style/chat_user_list.qss");
    qDebug() << "qss open" << f.open(QFile::ReadOnly) << f.errorString();
    const QString qss = QString::fromUtf8(f.readAll());
    sb->setStyleSheet(qss);

    // 像素滚动
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);

    viewport()->installEventFilter(this);

    m_scrollAnim = new QPropertyAnimation(verticalScrollBar(), "value", this);
    m_scrollAnim->setDuration(120);
    m_scrollAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_targetValue = verticalScrollBar()->value();

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this]{
        syncHoverUnderCursor();   // 每一帧滚动都刷新 hover
    });

}

void ChatUserList::smoothScrollTo(int target)
{
    QScrollBar *sb = verticalScrollBar();
    target = qBound(sb->minimum(), target, sb->maximum());

    m_targetValue = target;

    // 如果动画正在跑，先以“当前值”为起点继续滚
    if (m_scrollAnim->state() == QAbstractAnimation::Running) {
        m_scrollAnim->stop();
    }

    m_scrollAnim->setStartValue(sb->value());
    m_scrollAnim->setEndValue(m_targetValue);
    m_scrollAnim->start();
}

bool ChatUserList::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == this->viewport()) {
        // 触发 hover/leave 时让样式立即刷新
        if (event->type() == QEvent::Enter) {
            viewport()->update();
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);
        }else if(event->type() == QEvent::Leave){
            this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
        }
    }

        if (watched == this->viewport()&& event->type() == QEvent::Wheel) {
            auto *wheelEvent = static_cast<QWheelEvent*>(event);
            QScrollBar *sb = verticalScrollBar();
            int deltaPx = 0;

            if (!wheelEvent->pixelDelta().isNull()) {
                // 触控板
                deltaPx = wheelEvent->pixelDelta().y();
            } else {
                // 鼠标滚轮：按 item 高度比例滚动
                int itemH = 60;

                // 每格滚轮滚动 itemH 的 1.5
                const int stepPx = itemH * 1.55;
                deltaPx = (wheelEvent->angleDelta().y() / 120) * stepPx;
            }

            int target = m_targetValue - deltaPx;
            smoothScrollTo(target);

            // 到底加载更多
            if (m_targetValue >= sb->maximum()) {
                //加载更多用户处理
                emit sig_loading_chat_user();
            }
            return true;
        }
    //}
    return QListWidget::eventFilter(watched, event);
}
void ChatUserList::addChatUserWidget(QWidget *w)
{
    //  计算widget的布局
    w->setParent(this);
    w->adjustSize();
    w->updateGeometry();

    auto *item = new QListWidgetItem(this);

    // 强制用 widget 的 sizeHint 当 item 高度
    const int h = w->sizeHint().height();
    item->setSizeHint(QSize(0, h));

    addItem(item);
    setItemWidget(item, w);

    // 立刻刷新滚动范围
    refreshScrollRange();
}

void ChatUserList::refreshScrollRange()
{
    // 强制列表重新计算 item 布局和滚动条范围
    doItemsLayout();
    updateGeometries();
    viewport()->update();

    auto *sb = verticalScrollBar();
    qDebug() << "range" << sb->minimum() << sb->maximum()
             << "pageStep" << sb->pageStep()
             << "count" << count()
             << "viewportH" << viewport()->height();
}

void ChatUserList::syncHoverUnderCursor()
{
    QPoint vpPos = viewport()->mapFromGlobal(QCursor::pos());
    if (!viewport()->rect().contains(vpPos)) return;

    QMouseEvent fakeMove(QEvent::MouseMove,
                         vpPos,
                         Qt::NoButton,
                         Qt::NoButton,
                         Qt::NoModifier);
    QApplication::sendEvent(viewport(), &fakeMove);
}

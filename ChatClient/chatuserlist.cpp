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
#include <QTimer>
ChatUserList::ChatUserList(QWidget *parent)
    : QListWidget(parent)
{
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    setSelectionMode(QAbstractItemView::SingleSelection);

    // 用自定义短滚动条
    auto *sb = new ShortScrollBar(Qt::Vertical, this);
    setVerticalScrollBar(sb);

    setStyleSheet(R"(
    QListWidget { outline: 0px; }
    QListWidget::item:selected { background: transparent; border: none; }
    QListWidget::item { border: none; }

    QWidget#chat_user_item { background: transparent; border-radius: 6px; }
    QWidget#chat_user_item[hover="true"] { background: #EBEBEB; }
    QWidget#chat_user_item[selected="true"] { background: #E2E2E2; }

    ShortScrollBar {
    qproperty-barWidth: 6;
    qproperty-radius: 3;
    qproperty-handleLength: 18;

    qproperty-handleColor: rgba(0, 0, 0, 75);
    qproperty-handleHoverColor: rgba(0, 0, 0, 110);
    qproperty-handlePressedColor: rgba(0, 0, 0, 140);
    }
    )");
    // 像素滚动
    setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    setHorizontalScrollMode(QAbstractItemView::ScrollPerPixel);


    m_scrollAnim = new QPropertyAnimation(verticalScrollBar(), "value", this);
    m_scrollAnim->setDuration(120);
    m_scrollAnim->setEasingCurve(QEasingCurve::OutCubic);

    m_targetValue = verticalScrollBar()->value();

    setMouseTracking(true);
    viewport()->setMouseTracking(true);

    connect(verticalScrollBar(), &QScrollBar::valueChanged, this, [this](int v){
        m_targetValue = v;
        QTimer::singleShot(0, this, [this]{ updateHoverItem(); });
    });
    connect(this, &QListWidget::currentItemChanged, this,
            [this](QListWidgetItem* cur, QListWidgetItem* prev){
                auto setSel = [this](QListWidgetItem* it, bool sel){
                    if (!it) return;
                    if (auto *w = itemWidget(it)) {
                        w->setProperty("selected", sel);
                        w->style()->unpolish(w);
                        w->style()->polish(w);
                        w->update();
                    }
                };
                setSel(prev, false);
                setSel(cur, true);
            });

    connect(this, &QListWidget::itemClicked, this, [this](QListWidgetItem* item){
        if (!item) {
            m_pressedItem = nullptr;
            m_pressedItemWasSelected = false;
            return;
        }
        if (!(item->flags() & Qt::ItemIsSelectable)) {
            m_pressedItem = nullptr;
            m_pressedItemWasSelected = false;
            return;
        }
        if (item == m_pressedItem && m_pressedItemWasSelected) {
            setCurrentRow(-1);
            clearSelection();
            //执行条目取消选中后逻辑
            m_pressedItem = nullptr;
            m_pressedItemWasSelected = false;
            return;
        }
        if (item == m_pressedItem && !m_pressedItemWasSelected) {
            //执行条目选中后逻辑
        }
        m_pressedItem = nullptr;
        m_pressedItemWasSelected = false;
    });

    viewport()->installEventFilter(this);
}

void ChatUserList::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton) {
        QListWidgetItem *item = itemAt(event->pos());
        m_pressedItem = item;
        m_pressedItemWasSelected = item ? selectionModel()->isSelected(indexFromItem(item)) : false;
    } else {
        m_pressedItem = nullptr;
        m_pressedItemWasSelected = false;
    }
    QListWidget::mousePressEvent(event);
}

void ChatUserList::smoothScrollTo(int target)
{
    QScrollBar *sb = verticalScrollBar();
    target = qBound(sb->minimum(), target, sb->maximum());

    m_targetValue = target;

    // 以“当前值”为起点继续滚
    if (m_scrollAnim->state() == QAbstractAnimation::Running) {
        m_scrollAnim->stop();
    }

    m_scrollAnim->setStartValue(sb->value());
    m_scrollAnim->setEndValue(m_targetValue);
    m_scrollAnim->start();
}

void ChatUserList::scrollContentsBy(int dx, int dy)
{
    QListWidget::scrollContentsBy(dx, dy);
    if (m_hasLastVpPos) {
        QTimer::singleShot(0, this, [this]{ updateHoverItem(); });
    }
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
            clearHoverItem();
        }
    }

    if (watched == this->viewport() && event->type() == QEvent::Wheel) {

         auto *wheelEvent = static_cast<QWheelEvent*>(event);
        QScrollBar *sb = verticalScrollBar();

        m_lastVpPos = wheelEvent->position().toPoint();
        m_hasLastVpPos = true;
        updateHoverItemAt(m_lastVpPos);

        int deltaPx = 0;
        if (!wheelEvent->pixelDelta().isNull()) {
            // 触控板
            deltaPx = -wheelEvent->pixelDelta().y();
        } else {
               // 鼠标滚轮：按 item 高度比例滚动
             int itemH = 60;

            // 每格滚轮滚动 itemH 的 1.5
            const int stepPx = itemH * 1.55;
             deltaPx = -(wheelEvent->angleDelta().y() / 120) * stepPx;
         }

         int target = sb->value() + deltaPx;
        smoothScrollTo(target);
        updateHoverItemAt(m_lastVpPos);
         // 到底加载更多
        if (m_targetValue >= sb->maximum()) {
            //加载更多用户处理
             emit sig_loading_chat_user();
        }

        return true;
    }
    if (watched == viewport() && event->type() == QEvent::MouseMove) {
        auto *me = static_cast<QMouseEvent*>(event);
        m_lastVpPos = me->position().toPoint();
        m_hasLastVpPos = true;
        updateHoverItemAt(m_lastVpPos);
    }

    return QListWidget::eventFilter(watched, event);
}

void ChatUserList::clearHoverItem()
{
    if (!m_hoverItem) return;
    if (auto *w = itemWidget(m_hoverItem)) {
        w->setProperty("hover", false);
        w->style()->unpolish(w);
        w->style()->polish(w);
        w->update();
    }
    m_hoverItem = nullptr;
    m_hasLastVpPos = false;
}
void ChatUserList::addChatUserWidget(QWidget *w)
{
    //  计算widget的布局
    w->setParent(nullptr);
    w->updateGeometry();
    w->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    auto *item = new QListWidgetItem(this);

    // 强制用 widget 的 sizeHint 当 item 高度
    const int h = w->sizeHint().height();
    item->setSizeHint(QSize(this->viewport()->width(), h));

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
}



void ChatUserList::updateHoverItem()
{
    const QPoint vpPos = viewport()->mapFromGlobal(QCursor::pos());
    updateHoverItemAt(vpPos);
}

void ChatUserList::updateHoverItemAt(const QPoint& viewportPos)
{
    // viewportPos 是 viewport 坐标
    if (!viewport()->rect().contains(viewportPos)) return;

    QListWidgetItem *it = itemAt(viewportPos);
    if (it == m_hoverItem) return;

    // 清旧
    if (m_hoverItem) {
        if (auto *w = itemWidget(m_hoverItem)) {
            w->setProperty("hover", false);
            w->style()->unpolish(w); w->style()->polish(w);
            w->update();
        }
    }

    m_hoverItem = it;

    // 设新
    if (m_hoverItem) {
        if (auto *w = itemWidget(m_hoverItem)) {
            w->setProperty("hover", true);
            w->style()->unpolish(w); w->style()->polish(w);
            w->update();
        }
    }
}

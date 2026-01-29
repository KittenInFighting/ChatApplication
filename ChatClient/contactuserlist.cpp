#include "contactuserlist.h"
#include "grouptipitem.h"
#include "conuseritem.h"
#include "shortscrollbar.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include <QRandomGenerator>
#include <QMouseEvent>
#include <QCursor>
#include <QTimer>

ContactUserList::ContactUserList(QWidget *parent):_load_pending(false)
{
    Q_UNUSED(parent);

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

    viewport()->installEventFilter(this);


    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    // 安装事件过滤器
    this->viewport()->installEventFilter(this);

    //模拟从数据库或者后端传输过来的数据,进行列表加载
    addContactUserList();
    //连接点击的信号和槽
    connect(this, &QListWidget::itemClicked, this, &ContactUserList::slot_item_clicked);
       //链接对端同意认证后通知的信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_add_auth_friend,this,
               &ContactUserList::slot_add_auth_firend);

       //链接自己点击同意认证后界面刷新
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp,this,
               &ContactUserList::slot_auth_rsp);

}

void ContactUserList::mousePressEvent(QMouseEvent *event)
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

void ContactUserList::ShowRedPoint(bool bshow /*= true*/)
{
    // _add_friend_item->ShowRedPoint(bshow);
}

void ContactUserList::addContactUserList()
{
    auto * groupCon = new GroupTipItem();
    groupCon->SetGroupTip(tr("好友通知"));
    _groupitem = new QListWidgetItem;
    _groupitem->setSizeHint(groupCon->sizeHint());
    this->addItem(_groupitem);
    this->setItemWidget(_groupitem, groupCon);
    _groupitem->setFlags(_groupitem->flags() & ~Qt::ItemIsSelectable);

    //连接item按钮的点击事件
    connect(groupCon, &GroupTipItem::sig_switch_apply_friend_page,
            this, &ContactUserList::sig_switch_apply_friend_page);

    //加载后端发送过来的好友列表
    auto con_list = UserMgr::GetInstance()->GetConListPerPage();
    for(auto & con_ele : con_list){
        auto *con_user_wid = new ConUserItem();
        con_user_wid->SetInfo(con_ele->_uid,con_ele->_name, con_ele->_icon);
        QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(con_user_wid->sizeHint());
        this->addItem(item);
        this->setItemWidget(item, con_user_wid);
    }

    UserMgr::GetInstance()->UpdateContactLoadedCount();


    // 模拟创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *con_user_wid = new ConUserItem();
        con_user_wid->SetInfo(0,names[name_i], heads[head_i]);

        addContactUserWidget(con_user_wid);
        refreshScrollRange();
        //qDebug() <<"name = " << con_user_wid->GetInfo()->_name;
        //QListWidgetItem *item = new QListWidgetItem;
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        //item->setSizeHint(con_user_wid->sizeHint());
        //this->addItem(item);
        //this->setItemWidget(item, con_user_wid);
    }
}
void ContactUserList::addContactUserWidget(QWidget *w)
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

bool ContactUserList::eventFilter(QObject *watched, QEvent *event)
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
            //emit sig_loading_contact_user();

            auto b_loaded = UserMgr::GetInstance()->IsLoadConFin();
            if(b_loaded){
                return true;
            }

            if(_load_pending){
                return true;
            }

            _load_pending = true;

            QTimer::singleShot(100, [this](){
                _load_pending = false;
                QCoreApplication::quit(); // 完成后退出应用程序
            });
            // 滚动到底部，加载新的联系人
            qDebug()<<"load more contact user";
            //发送信号通知聊天界面加载更多聊天内容
            emit sig_loading_contact_user();
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

void ContactUserList::clearHoverItem()
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

void ContactUserList::slot_item_clicked(QListWidgetItem *item)
{
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
        qDebug()<< "contact user item clicked ";

        QWidget *widget = this->itemWidget(item); // 获取自定义widget对象
        if(!widget){
            qDebug()<< "slot item clicked widget is nullptr";
            m_pressedItem = nullptr;
            m_pressedItemWasSelected = false;
            return;
        }

        // 对自定义widget进行操作， 将item 转化为基类ListItemBase
        ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
        if(!customItem){
            qDebug()<< "slot item clicked widget is nullptr";
            m_pressedItem = nullptr;
            m_pressedItemWasSelected = false;
            return;
        }
        auto con_item = qobject_cast<ConUserItem*>(customItem);
        auto user_info = con_item->GetInfo();
        //跳转到好友信息界面
        emit sig_switch_friend_info_page(user_info);
        m_pressedItem = nullptr;
        m_pressedItemWasSelected = false;
        return;
    }
    m_pressedItem = nullptr;
    m_pressedItemWasSelected = false;
}

void ContactUserList::slot_add_auth_firend(std::shared_ptr<AuthInfo> auth_info)
{
    qDebug() << "slot add auth friend ";
    bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if(isFriend){
        return;
    }
    // 在 groupitem 之后插入新项
    //int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    //int head_i = randomValue%head.size();

    auto *con_user_wid = new ConUserItem();
    con_user_wid->SetInfo(auth_info);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(con_user_wid->sizeHint());

    // 获取 groupitem 的索引
    int index = this->row(_groupitem);
    // 在 groupitem 之后插入新项
    this->insertItem(index + 1, item);

    this->setItemWidget(item, con_user_wid);
}

void ContactUserList::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "slot auth rsp called";
    bool isFriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    if(isFriend){
        return;
    }
    // 在 groupitem 之后插入新项
    int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    int head_i = randomValue%heads.size();

    auto *con_user_wid = new ConUserItem();
    con_user_wid->SetInfo(auth_rsp->_uid ,auth_rsp->_name, heads[head_i]);
    QListWidgetItem *item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(con_user_wid->sizeHint());

    // 获取 groupitem 的索引
    int index = this->row(_groupitem);
    // 在 groupitem 之后插入新项
    this->insertItem(index + 1, item);

    this->setItemWidget(item, con_user_wid);
}

void ContactUserList::smoothScrollTo(int target)
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

void ContactUserList::scrollContentsBy(int dx, int dy)
{
    QListWidget::scrollContentsBy(dx, dy);
    if (m_hasLastVpPos) {
        QTimer::singleShot(0, this, [this]{ updateHoverItem(); });
    }
}

void ContactUserList::refreshScrollRange()
{
    // 强制列表重新计算 item 布局和滚动条范围
    doItemsLayout();
    updateGeometries();
    viewport()->update();
}



void ContactUserList::updateHoverItem()
{
    const QPoint vpPos = viewport()->mapFromGlobal(QCursor::pos());
    updateHoverItemAt(vpPos);
}

void ContactUserList::updateHoverItemAt(const QPoint& viewportPos)
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

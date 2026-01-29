#include "chatdialog.h"
#include "ui_chatdialog.h"
#include "addbtn.h"
#include "userdata.h"
#include "chatuserwid.h"
#include "findsuccessdlg.h"
#include "global.h"
#include "tcpmgr.h"
#include "usermgr.h"
#include "conuseritem.h"
#include <QMessageBox>
#include <QString>
#include <memory>
#include <QEvent>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QPixmap>
#include <QResizeEvent>
#include <QRegion>
#include <QStyle>
#include <QToolButton>
#include <QVBoxLayout>
#include <QRandomGenerator>
#ifdef Q_OS_WIN
#include <windows.h>
#include <dwmapi.h>
#include <windowsx.h>
#endif

namespace {
constexpr int kTitleBarHeight = 38;
constexpr int kWindowRadius = 10;

QPixmap makeRoundPixmap(const QPixmap &src, int size)
{
    if (src.isNull() || size <= 0) return QPixmap();

    const QPixmap scaled = src.scaled(size, size,
                                      Qt::KeepAspectRatioByExpanding,
                                      Qt::SmoothTransformation);
    QPixmap out(size, size);
    out.fill(Qt::transparent);

    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    p.setClipPath(path);
    const int x = (size - scaled.width()) / 2;
    const int y = (size - scaled.height()) / 2;
    p.drawPixmap(x, y, scaled);
    return out;
}
}

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false),_cur_chat_uid(0),_last_widget(nullptr)
{
    ui->setupUi(this);
    //设置窗口边框
    setWindowTitle("MyChat");
    setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_NoSystemBackground, true);
    setAttribute(Qt::WA_StyledBackground, true);
    initTitleBar();
    updateWindowMask();
    setMinimumHeight(minimumHeight() + kTitleBarHeight);
    ui->side_chat->SetState("normal", "hover", "pressed",
                            "pressed", "pressed_hover", "pressed");

    ui->side_contact->SetState("normal", "hover", "pressed",
                               "pressed", "pressed_hover", "pressed");

    ui->side_chat->setStyleSheet(R"(
    #side_chat{
     border: none;
     background: transparent;
     qproperty-iconSize: 20px 20px;
    }

    #side_chat[state='normal']{
     qproperty-icon: url(:/res/chat.png);
    }

    #side_chat[state='hover']{
     qproperty-icon: url(:/res/chat.png);
     background: rgba(0,0,0,0.06);
     border-radius: 6px;
    }

    #side_chat[state='pressed']{
     qproperty-icon: url(:/res/chat_press.png);
    }
    #side_chat[state='pressed_hover']{
     qproperty-icon: url(:/res/chat_press.png);
     background: rgba(0,0,0,0.06);
     border-radius: 6px;
    }
    )");

    ui->side_contact->setStyleSheet(R"(
    #side_contact{
     border: none;
     background: transparent;
     qproperty-iconSize: 20px 20px;
    }

    #side_contact[state='normal']{
     qproperty-icon: url(:/res/contact.png);
    }

    #side_contact[state='hover']{
     qproperty-icon: url(:/res/contact.png);
     background: rgba(0,0,0,0.06);
     border-radius: 6px;
    }

    #side_contact[state='pressed']{
     qproperty-icon: url(:/res/contact_press.PNG);
    }
    #side_contact[state='pressed_hover']{
     qproperty-icon: url(:/res/contact_press.PNG);
     background: rgba(0,0,0,0.06);
     border-radius: 6px;
    }
    )");
    AddLBGroup(ui->side_chat);
    AddLBGroup(ui->side_contact);

    connect(ui->side_chat, &StateWidget::clicked, this, &ChatDialog::slot_side_chat);
    connect(ui->side_contact, &StateWidget::clicked, this, &ChatDialog::slot_side_contact);

    ui->search_edit->SetMaxLength(30);
    QPixmap pm(":/res/search.png");
    pm = pm.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->search_edit->addAction(
        QIcon(":/res/search.png"),
        QLineEdit::LeadingPosition
        );
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));

    //启用背景绘制
    auto enableBg = [](QWidget* w){
        if (!w) return;
        w->setAttribute(Qt::WA_StyledBackground, true);
    };
    enableBg(this);//整个窗口
    enableBg(ui->side_bar);//左侧栏
    enableBg(ui->chat_user_wid); //左侧用户列表区域 + 搜索区
    enableBg(ui->search_wid);

    ui->chat_user_wid->setAttribute(Qt::WA_StyledBackground, true);
    ui->list_wid->setAttribute(Qt::WA_StyledBackground, true);

    //在list_wid和search_wid插入分割线
    QVBoxLayout* vlay = qobject_cast<QVBoxLayout*>(ui->chat_user_wid->layout());
    Q_ASSERT(vlay);

    vlay->setSpacing(0);
    vlay->setContentsMargins(0, 0, 0, 0);

    QFrame* hline = new QFrame(ui->chat_user_wid);
    hline->setFixedHeight(1);   //
    hline->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    hline->setStyleSheet("background:#E6E8EB;");

    // 插在 list_wid 和 search_wid 中间
    int idx = vlay->indexOf(ui->list_wid);
    vlay->insertWidget(idx, hline);

    // 创建Chat_user_wid的右侧分割线
    QFrame* vline = new QFrame(this);
    vline->setFixedWidth(1);
    vline->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);
    vline->setStyleSheet("background:#E6E8EB;");
    if (auto *bodyLay = qobject_cast<QHBoxLayout*>(ui->widget_2->layout())) {
        int chatIdx = bodyLay->indexOf(ui->chat_user_wid);
        if (chatIdx >= 0) {
            bodyLay->insertWidget(chatIdx + 1, vline);
        } else {
            bodyLay->addWidget(vline);
        }
    }

    //ChatDialog 的主布局
    QHBoxLayout* mainLay = qobject_cast<QHBoxLayout*>(this->layout());
    if (!mainLay) {
        qWarning() << "ChatDialog 没有 QHBoxLayout，检查 ui 设计";
        return;
    }

    //防止被 spacing/margin 挤掉
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, kTitleBarHeight, 0, 0);

    //右侧清空“×”action默认隐藏
    QAction* clearAct = ui->search_edit->addAction(
        QIcon(":/res/clearbtn.png"),
        QLineEdit::TrailingPosition
        );
    clearAct->setVisible(false);
    //有内容就显示，没内容隐藏
    connect(ui->search_edit, &QLineEdit::textChanged, this, [=](const QString& t){
        clearAct->setVisible(!t.isEmpty());
    });

    //点击“×”清空内容
    connect(clearAct, &QAction::triggered, this, [=](){
        ui->search_edit->clear();
        ui->search_edit->setFocus();
    });

    // 5) 把右侧 action 对应的按钮设置成不抢焦点
    QObjectList objs = ui->search_edit->children();
    for (int i = 0; i < objs.size(); ++i) {
        QToolButton* b = qobject_cast<QToolButton*>(objs.at(i));
        if (!b) continue;

        if (b->defaultAction() == clearAct) {
            b->setFocusPolicy(Qt::NoFocus);
            b->setCursor(Qt::ArrowCursor);
            b->setFixedSize(20, 20);        // hover 灰圆底大小
            b->setIconSize(QSize(12, 12));  // “×”本体大小
            break;
        }
    }

    //设置add_btn
    ui->add_btn->setPlusIcon(":/res/add_btn.png");

    ui->add_btn->setStyleSheet(R"(
    QPushButton#add_btn {
    background: #EBEBEB;
    border: 1px solid transparent;
    border-radius: 10px;
    width: 26px;
    height: 26px;
    min-width: 26px;
    max-width: 26px;
    min-height: 26px;
    max-height: 26px;
    padding: 0px;
    }
    QPushButton#add_btn:hover   { background: #E2E2E2; }
    QPushButton#add_btn:pressed { background: #CECECE; }
    )");

    ui->widget_2->setFocus(Qt::OtherFocusReason);//默认聚焦空白边框

    ShowList(false);

    //连接加载信号和槽
    connect(ui->chat_user_list, &ChatUserList::sig_loading_chat_user, this, &ChatDialog::slot_loading_chat_user);
    addChatUserList();
    //连接加载联系人的信号和槽函数
    connect(ui->contact_user_list, &ContactUserList::sig_loading_contact_user,
            this, &ChatDialog::slot_loading_contact_user);

    connect(ui->chat_user_list, &QListWidget::currentItemChanged, this,
            [=](QListWidgetItem* cur, QListWidgetItem* prev){
                if (prev) {
                    if (auto* w = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(prev))) {
                        w->setProperty("selected", false);
                        w->style()->unpolish(w);
                        w->style()->polish(w);
                    }
                }
                if (cur) {
                    if (auto* w = qobject_cast<ChatUserWid*>(ui->chat_user_list->itemWidget(cur))) {
                        w->setProperty("selected", true);
                        w->style()->unpolish(w);
                        w->style()->polish(w);
                    }
                }
            });

    this->installEventFilter(this);

    //连接搜索条目
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_user_search, this, &ChatDialog::slot_user_search);

    //搜索用户返回失败
    connect(TcpMgr::GetInstance().get(),&TcpMgr::sig_find_failed,this,&ChatDialog::slot_find_failed);

    //连接searchlist跳转聊天信号
    connect(this, &ChatDialog::sig_jump_chat_item, this, &ChatDialog::slot_jump_chat_item);

    //处理好友申请
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_friend_apply, this, &ChatDialog::slot_friend_apply);

    //连接认证添加好友信号
    connect(TcpMgr::GetInstance().get()  , &TcpMgr::sig_add_auth_friend, this, &ChatDialog::slot_add_auth_friend);

    //链接自己认证回复信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_auth_rsp, this, &ChatDialog::slot_auth_rsp);

    //连接点击联系人item发出的信号和用户信息展示槽函数
    connect(ui->contact_user_list, &ContactUserList::sig_switch_friend_info_page,
            this,&ChatDialog::slot_friend_info_page);

    //连接联系人页面点击好友申请条目的信号
    connect(ui->contact_user_list, &ContactUserList::sig_switch_apply_friend_page,
            this, &ChatDialog::slot_switch_apply_friend_page);

    //连接好友信息界面发送的点击事件
    connect(ui->friend_info_page, &FriendInfoPage::sig_jump_chat_item, this,
            &ChatDialog::slot_jump_chat_item_from_infopage);

    //连接聊天item的点击事件
    connect(ui->chat_user_list,&ChatUserList::sig_item_clicked,this,&ChatDialog::slot_chat_item_clicked);

    //连接聊天item的点击取消事件
    connect(ui->chat_user_list,&ChatUserList::sig_item_cancel,this,&ChatDialog::slot_chat_item_cancel);

    //连接对端消息通知
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_text_chat_msg, this, &ChatDialog::slot_text_chat_msg);
    //消息缓存
    connect(ui->chat_page, &ChatPage::sig_append_send_chat_msg, this, &ChatDialog::slot_append_send_chat_msg);


    //设置默认选中聊天界面
    ui->side_chat->SetSelected(true);
    //ui->stackedWidget->setCurrentWidget(ui->chat_page);
    ui->stackedWidget->setCurrentWidget(ui->white_page);
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

void ChatDialog::CloseFindDlg()
{
    if(_find_dlg){
        _find_dlg->hide();
        _find_dlg = nullptr;
    }
}

bool ChatDialog::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_titleBar) {
        if (event->type() == QEvent::MouseButtonPress) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                m_dragging = true;
                m_dragPos = me->globalPosition().toPoint() - frameGeometry().topLeft();
                return true;
            }
        } else if (event->type() == QEvent::MouseMove) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (m_dragging && (me->buttons() & Qt::LeftButton)) {
                const QPoint globalPos = me->globalPosition().toPoint();
                if (isMaximized()) {
                    const int w = width();
                    const qreal ratio = (w > 0) ? (me->position().x() / qreal(w)) : 0.5;
                    showNormal();
                    updateTitleButtons();
                    updateWindowMask();

                    const int newX = globalPos.x() - int(width() * ratio);
                    const int newY = globalPos.y() - int(me->position().y());
                    move(newX, newY);
                    m_dragPos = globalPos - frameGeometry().topLeft();
                }
                move(globalPos - m_dragPos);
                return true;
            }
        } else if (event->type() == QEvent::MouseButtonRelease) {
            m_dragging = false;
            return true;
        } else if (event->type() == QEvent::MouseButtonDblClick) {
            auto *me = static_cast<QMouseEvent *>(event);
            if (me->button() == Qt::LeftButton) {
                toggleMaxRestore();
                return true;
            }
        }
    }

    if (event->type() == QEvent::MouseButtonPress)
    {
        // 当前焦点在 search_edit 上才处理
        if (ui->search_edit->hasFocus())
        {
            // 如果点击的不是 search_edit
            QWidget *w = QApplication::widgetAt(QCursor::pos());
            if (w && (w == ui->search_edit || ui->search_edit->isAncestorOf(w)))
                return QDialog::eventFilter(obj, event);

            ui->search_edit->clearFocus();
        }
    }
    // 继续交给父类处理
    return QDialog::eventFilter(obj, event);
}

void ChatDialog::initTitleBar()
{
    m_titleBar = new QWidget(this);
    m_titleBar->setObjectName("chat_title_bar");
    m_titleBar->setFixedHeight(kTitleBarHeight);
    m_titleBar->setAttribute(Qt::WA_StyledBackground, true);
    m_titleBar->setStyleSheet(
        "#chat_title_bar{background:#EBEBEB;}"
        "QToolButton{border:none;background:transparent;border-radius:6px;}"
        "QToolButton:hover{background:#BFBFBF;}"
        );

    auto *layout = new QHBoxLayout(m_titleBar);
    layout->setContentsMargins(10, 0, 6, 0);
    layout->setSpacing(4);

    m_titleLabel = new QLabel(windowTitle(), m_titleBar);
    m_titleLabel->setObjectName("chat_title_label");
    m_titleLabel->setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    m_titleLabel->setStyleSheet("color:#202020;font-size:12px;");
    m_titleLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    layout->addWidget(m_titleLabel);

    auto *info = new QWidget(m_titleBar);
    info->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto *infoLayout = new QHBoxLayout(info);
    infoLayout->setContentsMargins(10, 0, 10, 0);
    infoLayout->setSpacing(6);

    const int avatarSize = 24;
    auto *avatar = new QLabel(info);
    avatar->setFixedSize(avatarSize, avatarSize);
    avatar->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    avatar->setPixmap(makeRoundPixmap(QPixmap(":/res/1.jpg"), avatarSize));
    avatar->setScaledContents(false);

    auto *textWrap = new QWidget(info);
    textWrap->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    auto *textLayout = new QHBoxLayout(textWrap);
    textLayout->setContentsMargins(0, 0, 0, 0);
    textLayout->setSpacing(8);

    auto *nameLabel = new QLabel(QStringLiteral("牡牡蛎"), textWrap);
    nameLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    nameLabel->setStyleSheet("color:#202020;font-size:14px;");

    auto *signLabel = new QLabel(QStringLiteral("事缓则圆，人缓则安"), textWrap);
    signLabel->setAttribute(Qt::WA_TransparentForMouseEvents, true);
    signLabel->setStyleSheet("color:#6B6B6B;font-size:11px;");

    textLayout->addWidget(nameLabel);
    textLayout->addWidget(signLabel);

    infoLayout->addWidget(avatar);
    infoLayout->addWidget(textWrap);
    layout->addWidget(info);
    layout->addStretch();

    m_minBtn = new QToolButton(m_titleBar);
    m_minBtn->setObjectName("chat_min_btn");
    m_minBtn->setFixedSize(40, kTitleBarHeight - 2);
    m_minBtn->setFocusPolicy(Qt::NoFocus);
    m_minBtn->setIcon(QIcon(":/res/minus.png"));
    m_minBtn->setIconSize(QSize(18, 18));
    connect(m_minBtn, &QToolButton::clicked, this, &QWidget::showMinimized);

    m_maxBtn = new QToolButton(m_titleBar);
    m_maxBtn->setObjectName("chat_max_btn");
    m_maxBtn->setFixedSize(40, kTitleBarHeight - 2);
    m_maxBtn->setFocusPolicy(Qt::NoFocus);
    connect(m_maxBtn, &QToolButton::clicked, this, &ChatDialog::toggleMaxRestore);

    m_closeBtn = new QToolButton(m_titleBar);
    m_closeBtn->setObjectName("chat_close_btn");
    m_closeBtn->setFixedSize(40, kTitleBarHeight - 2);
    m_closeBtn->setFocusPolicy(Qt::NoFocus);
    m_closeBtn->setIcon(QIcon(":/res/close.png"));
    m_closeBtn->setIconSize(QSize(8, 8));
    connect(m_closeBtn, &QToolButton::clicked, this, &QWidget::close);

    layout->addWidget(m_minBtn);
    layout->addWidget(m_maxBtn);
    layout->addWidget(m_closeBtn);

    updateTitleButtons();
    m_titleBar->setGeometry(0, 0, width(), kTitleBarHeight);
    m_titleBar->raise();
    m_titleBar->installEventFilter(this);
}

void ChatDialog::updateTitleButtons()
{
    if (!m_maxBtn) return;
    if (isMaximized()) {
        m_maxBtn->setIcon(QIcon(":/res/max2.png"));
        m_maxBtn->setIconSize(QSize(18, 18));
    } else {
        m_maxBtn->setIcon(QIcon(":/res/max.png"));
        m_maxBtn->setIconSize(QSize(14, 14));
    }

}

void ChatDialog::toggleMaxRestore()
{
    if (isMaximized()) {
        showNormal();
    } else {
        showMaximized();
    }
    updateTitleButtons();
    updateWindowMask();
}

bool ChatDialog::hitTitleBar(const QPoint &pos) const
{
    if (!m_titleBar || pos.y() >= kTitleBarHeight) return false;

    const QPoint localPos = m_titleBar->mapFromParent(pos);
    if (m_minBtn && m_minBtn->geometry().contains(localPos)) return false;
    if (m_maxBtn && m_maxBtn->geometry().contains(localPos)) return false;
    if (m_closeBtn && m_closeBtn->geometry().contains(localPos)) return false;

    return true;
}

void ChatDialog::updateWindowMask()
{
    clearMask();
#ifdef Q_OS_WIN
    const int kAttr = 33; // DWMWA_WINDOW_CORNER_PREFERENCE
    const int kDoNotRound = 1; // DWMWCP_DONOTROUND
    const int kRound = 2; // DWMWCP_ROUND
    const int pref = isMaximized() ? kDoNotRound : kRound;
    DwmSetWindowAttribute(reinterpret_cast<HWND>(winId()), kAttr, &pref, sizeof(pref));
#endif
}

void ChatDialog::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setPen(Qt::NoPen);

    const QColor bg(245, 245, 245);
    if (isMaximized()) {
        p.setBrush(bg);
        p.drawRect(rect());
        return;
    }

    QRectF r = rect();
    r.adjust(0.5, 0.5, -0.5, -0.5);
    QPainterPath path;
    path.addRoundedRect(r, kWindowRadius, kWindowRadius);
    p.fillPath(path, bg);
}

void ChatDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);
    if (m_titleBar) {
        m_titleBar->setGeometry(0, 0, width(), kTitleBarHeight);
    }
}

void ChatDialog::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && hitTitleBar(event->pos())) {
        m_dragging = true;
        m_dragPos = event->globalPosition().toPoint() - frameGeometry().topLeft();
        event->accept();
        return;
    }
    QDialog::mousePressEvent(event);
}

void ChatDialog::mouseMoveEvent(QMouseEvent *event)
{
    if (m_dragging && (event->buttons() & Qt::LeftButton)) {
        move(event->globalPosition().toPoint() - m_dragPos);
        event->accept();
        return;
    }
    QDialog::mouseMoveEvent(event);
}

void ChatDialog::mouseReleaseEvent(QMouseEvent *event)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(event);
}

void ChatDialog::mouseDoubleClickEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton && hitTitleBar(event->pos())) {
        toggleMaxRestore();
        event->accept();
        return;
    }
    QDialog::mouseDoubleClickEvent(event);
}

void ChatDialog::changeEvent(QEvent *event)
{
    QDialog::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        updateTitleButtons();
        updateWindowMask();
    } else if (event->type() == QEvent::WindowTitleChange) {
        if (m_titleLabel) {
            m_titleLabel->setText(windowTitle());
        }
    }
}

bool ChatDialog::nativeEvent(const QByteArray &eventType, void *message, qintptr *result)
{
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG" || eventType == "windows_dispatcher_MSG") {
        auto *msg = static_cast<MSG *>(message);
        if (msg->message == WM_NCHITTEST) {
            if (isMaximized()) {
                return QDialog::nativeEvent(eventType, message, result);
            }

            const int kResizeBorder = 6;
            const int x = GET_X_LPARAM(msg->lParam);
            const int y = GET_Y_LPARAM(msg->lParam);
            RECT winRect;
            GetWindowRect(reinterpret_cast<HWND>(winId()), &winRect);

            const bool onLeft = x >= winRect.left && x < winRect.left + kResizeBorder;
            const bool onRight = x <= winRect.right && x > winRect.right - kResizeBorder;
            const bool onTop = y >= winRect.top && y < winRect.top + kResizeBorder;
            const bool onBottom = y <= winRect.bottom && y > winRect.bottom - kResizeBorder;

            if (onTop && onLeft) {
                *result = HTTOPLEFT;
                return true;
            }
            if (onTop && onRight) {
                *result = HTTOPRIGHT;
                return true;
            }
            if (onBottom && onLeft) {
                *result = HTBOTTOMLEFT;
                return true;
            }
            if (onBottom && onRight) {
                *result = HTBOTTOMRIGHT;
                return true;
            }
            if (onLeft) {
                *result = HTLEFT;
                return true;
            }
            if (onRight) {
                *result = HTRIGHT;
                return true;
            }
            if (onTop) {
                *result = HTTOP;
                return true;
            }
            if (onBottom) {
                *result = HTBOTTOM;
                return true;
            }
        }
    }
#endif
    return QDialog::nativeEvent(eventType, message, result);
}

void ChatDialog::addChatUserList()
{
    //先按照好友列表加载聊天记录，等以后客户端实现聊天记录数据库之后再按照最后信息排序
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for(auto & friend_ele : friend_list){
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if(find_iter != _chat_items_added.end()){
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            //item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addChatUserWidget(chat_user_wid,item);
            ui->chat_user_list->refreshScrollRange();
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }

    //模拟聊天QListWidgetItem，并设置自定义的widget
    // for(int i = 0; i < 30; i++){
    //     int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
    //     int str_i = randomValue%strs.size();
    //     int head_i = randomValue%heads.size();
    //     int name_i = randomValue%names.size();
    //     int time_i = randomValue%times.size();
    //     auto *chat_user_wid = new ChatUserWid();
    //     QListWidgetItem *item = new QListWidgetItem;
    //     chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i], times[time_i]);
    //     ui->chat_user_list->addChatUserWidget(chat_user_wid,item);
    //     ui->chat_user_list->refreshScrollRange();
    // }
}


void ChatDialog::ShowList(bool bsearch)
{
    if(bsearch){
        ui->chat_user_list->hide();
        ui->contact_user_list->hide();
        ui->search_user_list->show();
        _mode = ChatUIMode::SearchMode;
    }else if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->contact_user_list->hide();
        ui->search_user_list->hide();
        _mode = ChatUIMode::ChatMode;
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        ui->search_user_list->hide();
        ui->contact_user_list->show();
        _mode = ChatUIMode::ContactMode;
    }
}

void ChatDialog::slot_side_chat()
{
    qDebug()<< "receive side chat clicked";
    ClearLabelState(ui->side_chat);
    if(_has_chat_context == true){
        ui->stackedWidget->setCurrentWidget(ui->chat_page);
    }
    else{
        ui->stackedWidget->setCurrentWidget(ui->white_page);
    }
    _state = ChatUIMode::ChatMode;
    ShowSearch();
}

void ChatDialog::slot_side_contact()
{
    qDebug()<< "receive side contact clicked";
    ui->side_contact->ShowRedPoint(false);
    ClearLabelState(ui->side_contact);
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
    _state = ChatUIMode::ContactMode;
    ShowSearch();
}

void ChatDialog::ClearLabelState(StateWidget *lb)
{
    for(auto & ele: _lb_list){
        if(ele == lb){
            continue;
        }

        ele->ClearState();
    }
}

void ChatDialog::AddLBGroup(StateWidget *lb)
{
    _lb_list.push_back(lb);
}

void ChatDialog::ShowSearch()
{

    if(_state == ChatUIMode::ChatMode){
        ui->chat_user_list->show();
        ui->contact_user_list->hide();
        //ui->search_list->hide();
        _mode = ChatUIMode::ChatMode;
        //ui->search_list->CloseFindDlg();
        ui->search_edit->clear();
        ui->search_edit->clearFocus();
    }else if(_state == ChatUIMode::ContactMode){
        ui->chat_user_list->hide();
        //ui->search_list->hide();
        ui->contact_user_list->show();
        _mode = ChatUIMode::ContactMode;
        //ui->search_list->CloseFindDlg();
        ui->search_edit->clear();
        ui->search_edit->clearFocus();
    }
}

void ChatDialog::on_add_btn_clicked()
{
    if(ui->search_edit->text().isEmpty()){
        QMessageBox::information(this, tr("提示"), tr("搜索用户不能为空！！！"));
        return;
    }
    auto uid_str = ui->search_edit->text();
    QJsonObject jsonObj;
    jsonObj["uid"] = uid_str;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_SEARCH_USER_REQ,
                                              jsonData);

    return;

}

void ChatDialog::slot_user_search(std::shared_ptr<SearchInfo> si)
{
    if(si == nullptr){
        QMessageBox::information(this, tr("提示"), tr("该用户不存在！！！"));
    }else{
        //如果是自己，暂且先直接返回，以后看逻辑扩充
        auto self_uid = UserMgr::GetInstance()->GetUid();
        if (si->_uid == self_uid) {
            QMessageBox::information(this, tr("提示"), tr("不能添加自己！！！"));
            return;
        }
        //此处分两种情况，一种是搜到已经是自己的朋友了，一种是未添加好友
        //查找是否已经是好友
        bool bExist = UserMgr::GetInstance()->CheckFriendById(si->_uid);
        if(bExist){
            //此处处理已经添加的好友，实现页面跳转
            //发送提示窗口或者直接跳转
            emit sig_jump_chat_item(si);
            //QMessageBox::information(this, tr("提示"), tr("已经是好友！！！"));
            return;
        }
        //此处处理添加的好友
        _find_dlg = std::make_shared<FindSuccessDlg>(this);
        std::dynamic_pointer_cast<FindSuccessDlg>(_find_dlg)->SetSearchInfo(si);
        _find_dlg -> show();
    }
}

void ChatDialog::slot_find_failed()
{
    QMessageBox::information(this, tr("提示"), tr("该用户不存在！！！"));
    return;
}

void ChatDialog::slot_friend_apply(std::shared_ptr<AddFriendApply> apply)
{
    qDebug() << "receive apply friend slot, applyuid is " << apply->_from_uid << " name is "
             << apply->_name << " desc is " << apply->_desc;

    bool b_already = UserMgr::GetInstance()->AlreadyApply(apply->_from_uid);
    if(b_already){
        return;
    }

    UserMgr::GetInstance()->AddApplyList(std::make_shared<ApplyInfo>(apply));
    ui->side_contact->ShowRedPoint(true);
    ui->friend_apply_page->AddNewApply(apply);
}

void ChatDialog::slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp)
{
    qDebug() << "receive slot_auth_rsp uid is " << auth_rsp->_uid
             << " name is " << auth_rsp->_name << " nick is " << auth_rsp->_nick;

    UserMgr::GetInstance()->UpdateApplyStatus(auth_rsp->_uid, 1);
    RefreshApplyRedPoint();

    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_rsp->_uid);
    if(bfriend){
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_rsp);

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_rsp);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_rsp->_uid, item);
}

void ChatDialog::slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info)
{
    qDebug() << "receive slot_add_auth__friend uid is " << auth_info->_uid
             << " name is " << auth_info->_name << " nick is " << auth_info->_nick;

    //判断如果已经是好友则跳过
    auto bfriend = UserMgr::GetInstance()->CheckFriendById(auth_info->_uid);
    if(bfriend){
        return;
    }

    UserMgr::GetInstance()->AddFriend(auth_info);

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(auth_info);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(auth_info->_uid, item);
}

void ChatDialog::SetSelectChatItem(int uid)
{
    if(ui->chat_user_list->count() <= 0){
        return;
    }

    if(uid == 0){
        ui->chat_user_list->setCurrentRow(0);
        QListWidgetItem *firstItem = ui->chat_user_list->item(0);
        if(!firstItem){
            return;
        }

        //转为widget
        QWidget *widget = ui->chat_user_list->itemWidget(firstItem);
        if(!widget){
            return;
        }

        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if(!con_item){
            return;
        }

        _cur_chat_uid = con_item->GetUserInfo()->_uid;

        return;
    }

    auto find_iter = _chat_items_added.find(uid);
    if(find_iter == _chat_items_added.end()){
        qDebug() << "uid " <<uid<< " not found, set curent row 0";
        ui->chat_user_list->setCurrentRow(0);
        return;
    }

    ui->chat_user_list->setCurrentItem(find_iter.value());

    _cur_chat_uid = uid;
}

void ChatDialog::SetSelectChatPage(int uid)
{
    if( ui->chat_user_list->count() <= 0){
        return;
    }

    if (uid == 0) {
        auto item = ui->chat_user_list->item(0);
        //转为widget
        QWidget* widget = ui->chat_user_list->itemWidget(item);
        if (!widget) {
            return;
        }

        auto con_item = qobject_cast<ChatUserWid*>(widget);
        if (!con_item) {
            return;
        }

        //设置信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);
        return;
    }

    auto find_iter = _chat_items_added.find(uid);
    if(find_iter == _chat_items_added.end()){
        return;
    }

    //转为widget
    QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
    if(!widget){
        return;
    }

    //判断转化为自定义的widget
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if(itemType == CHAT_USER_ITEM){
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if(!con_item){
            return;
        }

        //设置信息
        auto user_info = con_item->GetUserInfo();
        ui->chat_page->SetUserInfo(user_info);

        return;
    }

}

void ChatDialog::loadMoreChatUser()
{
    auto friend_list = UserMgr::GetInstance()->GetChatListPerPage();
    if (friend_list.empty() == false) {
        for(auto & friend_ele : friend_list){
            auto find_iter = _chat_items_added.find(friend_ele->_uid);
            if(find_iter != _chat_items_added.end()){
                continue;
            }
            auto *chat_user_wid = new ChatUserWid();
            auto user_info = std::make_shared<UserInfo>(friend_ele);
            chat_user_wid->SetInfo(user_info);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->chat_user_list->addItem(item);
            ui->chat_user_list->setItemWidget(item, chat_user_wid);
            _chat_items_added.insert(friend_ele->_uid, item);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateChatLoadedCount();
    }
}

void ChatDialog::loadMoreConUser()
{
    auto friend_list = UserMgr::GetInstance()->GetConListPerPage();
    if (friend_list.empty() == false) {
        for(auto & friend_ele : friend_list){
            auto *chat_user_wid = new ConUserItem();
            chat_user_wid->SetInfo(friend_ele->_uid,friend_ele->_name,
                                   friend_ele->_icon);
            QListWidgetItem *item = new QListWidgetItem;
            //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
            item->setSizeHint(chat_user_wid->sizeHint());
            ui->contact_user_list->addItem(item);
            ui->contact_user_list->setItemWidget(item, chat_user_wid);
        }

        //更新已加载条目
        UserMgr::GetInstance()->UpdateContactLoadedCount();
    }
}
void ChatDialog::slot_jump_chat_item(std::shared_ptr<SearchInfo> si)
{
    qDebug() << "slot jump chat item " << "\n";
    auto find_iter = _chat_items_added.find(si->_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "jump to chat item , uid is " << si->_uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat->SetSelected(true);
        SetSelectChatItem(si->_uid);
        //更新聊天界面信息
        SetSelectChatPage(si->_uid);
        slot_side_chat();
        return;
    }

    //如果没找到，则创建新的插入listwidget

    auto* chat_user_wid = new ChatUserWid();
    auto user_info = std::make_shared<UserInfo>(si);
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(si->_uid, item);

    ui->side_chat->SetSelected(true);
    SetSelectChatItem(si->_uid);
    //更新聊天界面信息
    SetSelectChatPage(si->_uid);
    _has_chat_context = true;
    slot_side_chat();
}

void ChatDialog::slot_loading_chat_user()
{
    qDebug() << "add new data to list.....";
    loadMoreChatUser();
}

void ChatDialog::slot_loading_contact_user()
{
    qDebug() << "add new data to list.....";
    loadMoreConUser();
}

void ChatDialog::RefreshApplyRedPoint()
{
    const bool has_apply = UserMgr::GetInstance()->HasPendingApply();
    ui->side_contact->ShowRedPoint(has_apply);
}


void ChatDialog::slot_friend_info_page(std::shared_ptr<UserInfo> user_info)
{
    qDebug()<<"receive switch friend info page sig";
    _last_widget = ui->friend_info_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_info_page);
    ui->friend_info_page->SetInfo(user_info);
}

void ChatDialog::slot_switch_apply_friend_page()
{
    qDebug()<<"receive switch apply friend page sig";
    _last_widget = ui->friend_apply_page;
    ui->stackedWidget->setCurrentWidget(ui->friend_apply_page);
}

void ChatDialog::slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info)
{
    qDebug() << "slot jump chat item " << "\n";
    auto find_iter = _chat_items_added.find(user_info->_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "jump to chat item , uid is " << user_info->_uid;
        ui->chat_user_list->scrollToItem(find_iter.value());
        ui->side_chat->SetSelected(true);
        SetSelectChatItem(user_info->_uid);
        //更新聊天界面信息
        SetSelectChatPage(user_info->_uid);
        _has_chat_context = true;
        slot_side_chat();
        return;
    }

    //如果没找到，则创建新的插入listwidget

    auto* chat_user_wid = new ChatUserWid();
    chat_user_wid->SetInfo(user_info);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);

    _chat_items_added.insert(user_info->_uid, item);

    ui->side_chat->SetSelected(true);
    SetSelectChatItem(user_info->_uid);
    //更新聊天界面信息
    SetSelectChatPage(user_info->_uid);
    _has_chat_context = true;
    slot_side_chat();
}

void ChatDialog::slot_chat_item_clicked(QListWidgetItem *item)
{
    QWidget *widget = ui->chat_user_list->itemWidget(item); // 获取自定义widget对象
    if(!widget){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }

    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase *customItem = qobject_cast<ListItemBase*>(widget);
    if(!customItem){
        qDebug()<< "slot item clicked widget is nullptr";
        return;
    }
    auto itemType = customItem->GetItemType();
    if(itemType == ListItemType::CHAT_USER_ITEM){
        // 创建对话框，提示用户
        qDebug()<< "chat user item clicked ";

        auto chat_wid = qobject_cast<ChatUserWid*>(customItem);
        auto user_info = chat_wid->GetUserInfo();
        //跳转到聊天界面
        ui->chat_page->SetUserInfo(user_info);
        _cur_chat_uid = user_info->_uid;
        _has_chat_context = true;
        ui->side_chat->SetSelected(true);
        slot_side_chat();
        return;
    }
}

void ChatDialog::slot_chat_item_cancel()
{
    _has_chat_context = false;
    slot_side_chat();
}

void ChatDialog::slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata)
{
    if (_cur_chat_uid == 0) {
        return;
    }

    auto find_iter = _chat_items_added.find(_cur_chat_uid);
    if (find_iter == _chat_items_added.end()) {
        return;
    }

    //转为widget
    QWidget* widget = ui->chat_user_list->itemWidget(find_iter.value());
    if (!widget) {
        return;
    }

    //判断转化为自定义的widget
    // 对自定义widget进行操作， 将item 转化为基类ListItemBase
    ListItemBase* customItem = qobject_cast<ListItemBase*>(widget);
    if (!customItem) {
        qDebug() << "qobject_cast<ListItemBase*>(widget) is nullptr";
        return;
    }

    auto itemType = customItem->GetItemType();
    if (itemType == CHAT_USER_ITEM) {
        auto con_item = qobject_cast<ChatUserWid*>(customItem);
        if (!con_item) {
            return;
        }

        //设置信息
        auto user_info = con_item->GetUserInfo();
        user_info->_chat_msgs.push_back(msgdata);
        std::vector<std::shared_ptr<TextChatData>> msg_vec;
        msg_vec.push_back(msgdata);
        UserMgr::GetInstance()->AppendFriendChatMsg(_cur_chat_uid,msg_vec);
        return;
    }
}
void ChatDialog::UpdateChatMsg(std::vector<std::shared_ptr<TextChatData> > msgdata)
{
    for(auto & msg : msgdata){
        if(msg->_from_uid != _cur_chat_uid){
            break;
        }

        ui->chat_page->AppendChatMsg(msg);
    }
}

void ChatDialog::slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg)
{
    auto find_iter = _chat_items_added.find(msg->_from_uid);
    if(find_iter != _chat_items_added.end()){
        qDebug() << "set chat item msg, uid is " << msg->_from_uid;
        QWidget *widget = ui->chat_user_list->itemWidget(find_iter.value());
        auto chat_wid = qobject_cast<ChatUserWid*>(widget);
        if(!chat_wid){
            return;
        }
        chat_wid->updateLastMsg(msg->_chat_msgs);
        //更新当前聊天页面记录
        UpdateChatMsg(msg->_chat_msgs);
        UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
        return;
    }

    //如果没找到，则创建新的插入listwidget
    auto* chat_user_wid = new ChatUserWid();
    //查询好友信息
    auto fi_ptr = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
    chat_user_wid->SetInfo(fi_ptr);
    QListWidgetItem* item = new QListWidgetItem;
    //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
    item->setSizeHint(chat_user_wid->sizeHint());
    chat_user_wid->updateLastMsg(msg->_chat_msgs);
    UserMgr::GetInstance()->AppendFriendChatMsg(msg->_from_uid,msg->_chat_msgs);
    ui->chat_user_list->insertItem(0, item);
    ui->chat_user_list->setItemWidget(item, chat_user_wid);
    _chat_items_added.insert(msg->_from_uid, item);
}

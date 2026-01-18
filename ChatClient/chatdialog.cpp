#include "chatdialog.h"
#include "ui_chatdialog.h"
#include "addbtn.h"
#include "chatuserwid.h"
#include <QString>
#include <QToolButton>
#include <QRandomGenerator>

std::vector<QString>  strs ={"hello !00000000000000",
                             "nice to meet u0000000000000",
                             "Ohiyo0000000000000000",
                             "Dear00000000000000",
                             "My Honey00000000000000"};

std::vector<QString> heads = {
    ":/res/1.jpg",
};

std::vector<QString> names = {
    "Mihariooooooooo",
    "Mahiro0000000000",
    "Asahi00000000000",
    "Momiji000000000",
    "Kade00000000000",
};

std::vector<QString> times = {
    "2025/06/08",
    "9:30",
    "8:00",
    "7:30",
    "13:00",
};
ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false)
{
    ui->setupUi(this);
    //设置窗口边框
    setWindowFlags(Qt::Window
                   | Qt::WindowMinimizeButtonHint
                   | Qt::WindowMaximizeButtonHint
                   | Qt::WindowCloseButtonHint);
    show();

    ui->side_chat->SetState("normal", "hover", "pressed",
                            "pressed", "pressed", "pressed");

    ui->side_contact->SetState("normal", "hover", "pressed",
                               "pressed", "pressed", "pressed");

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

    //ChatDialog 的主布局
    QHBoxLayout* mainLay = qobject_cast<QHBoxLayout*>(this->layout());
    if (!mainLay) {
        qWarning() << "ChatDialog 没有 QHBoxLayout，检查 ui 设计";
        return;
    }

    //防止被 spacing/margin 挤掉
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

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

    addChatUserList();
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
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

bool ChatDialog::eventFilter(QObject *obj, QEvent *event)
{
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

void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 30; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();
        int time_i = randomValue%times.size();
        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i], times[time_i]);
        ui->chat_user_list->addChatUserWidget(chat_user_wid);
        ui->chat_user_list->refreshScrollRange();
    }
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
    ui->stackedWidget->setCurrentWidget(ui->chat_page);
    _state = ChatUIMode::ChatMode;
    ShowSearch();
}

void ChatDialog::slot_side_contact()
{
    qDebug()<< "receive side contact clicked";
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

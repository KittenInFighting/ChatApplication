#include "chatdialog.h"
#include "InputTextEdit.h"
#include "ui_chatdialog.h"
#include "addbtn.h"
#include "chatuserwid.h"
#include <QString>
#include <QToolButton>
#include <QRandomGenerator>

std::vector<QString>  strs ={"hello !",
                             "nice to meet u",
                             "Ohiyo",
                             "Dear",
                             "My Honey"};

std::vector<QString> heads = {
    ":/res/1.jpg",
};

std::vector<QString> names = {
    "Mihari",
    "Mahiro",
    "Asahi",
    "Momiji",
    "Kade",
};

ChatDialog::ChatDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ChatDialog),_mode(ChatUIMode::ChatMode),
    _state(ChatUIMode::ChatMode),_b_loading(false)
{
    ui->setupUi(this);

    ui->search_edit->SetMaxLength(30);
    QPixmap pm(":/res/search.png");
    pm = pm.scaled(15, 15, Qt::KeepAspectRatio, Qt::SmoothTransformation);
    ui->search_edit->addAction(
        QIcon(":/res/search.png"),
        QLineEdit::LeadingPosition
        );
    ui->search_edit->setPlaceholderText(QStringLiteral("搜索"));
    // 点击空白处，让 search_edit 失焦
    ui->chat_data_wid->installEventFilter(this);

    //启用背景绘制
    auto enableBg = [](QWidget* w){
        if (!w) return;
        w->setAttribute(Qt::WA_StyledBackground, true);
    };
    enableBg(this);//整个窗口
    enableBg(ui->side_bar);//左侧栏
    enableBg(ui->chat_user_wid); //左侧用户列表区域 + 搜索区
    enableBg(ui->search_wid);
    enableBg(ui->chat_data_wid);//右侧聊天区域
    enableBg(ui->mode_wid);
    ui->chat_user_wid->setAttribute(Qt::WA_StyledBackground, true);
    ui->title_wid->setAttribute(Qt::WA_StyledBackground, true);
    ui->title_wid->setAttribute(Qt::WA_StyleSheetTarget, true);
    ui->mode_wid->setAttribute(Qt::WA_StyledBackground, true);
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

    // 把分割线插入
    int idx2 = mainLay->indexOf(ui->chat_data_wid);
    if (idx2 >= 0) {
        mainLay->insertWidget(idx2, vline);
    }

    //防止被 spacing/margin 挤掉
    mainLay->setSpacing(0);
    mainLay->setContentsMargins(0, 0, 0, 0);

    //右侧清空“×”action默认隐藏
    QAction* clearAct = ui->search_edit->addAction(
        QIcon(":/res/clearbtn.png"),   // 你可以换成系统自带或你自己的×图标
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

    // 5) 把右侧 action 对应的按钮设置成“不抢焦点/尺寸合适”
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
    //取出输入框TextEdit 指针
    auto te = ui->textEdit;
    //创建发送按钮，并“叠加”到 textEdit 内部
    sendBtn = new QPushButton(QStringLiteral("发送"), te);
    sendBtn->setStyleSheet(R"(
    QPushButton{
        background:#1E88FF;          /* 蓝色背景 */
        border:none;
        border-radius:10px;
        color:#FFFFFF;               /* 有内容：白字 */
        font-size:14px;
        padding:0px 12px;
    }
    QPushButton:disabled{
        color:rgba(255,255,255,0.55); /* 无内容：字体变浅（图2效果） */
    }
    )");
    //固定大小
    sendBtn->setFixedSize(100, 33);
    //显示在最上层
    sendBtn->raise();
    // 右下角留空隙
    const int padding = 10;
    te->setOverlayMargins(sendBtn->width() + padding, sendBtn->height() + padding);
    //初次定位按钮
    repositionSendBtn();
    //textEdit 变化大小时，重新定位按钮
    connect(te, &InputTextEdit::resized, this, &ChatDialog::repositionSendBtn);
    auto updateSendBtnState = [=]() {
        const bool hasContent = !te->toPlainText().trimmed().isEmpty();
        sendBtn->setEnabled(hasContent);
    };
    updateSendBtnState();
    // 输入变化时实时更新
    connect(te, &QTextEdit::textChanged, this, updateSendBtnState);
    //点击发送
    connect(sendBtn, &QPushButton::clicked, this, &ChatDialog::onSendClicked);

    //设置add_btn
    ui->add_btn->setPlusIcon(":/res/add_btn.png");

    ShowList(false);

    addChatUserList();
}

ChatDialog::~ChatDialog()
{
    delete ui;
}

bool ChatDialog::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->textEdit && event->type() == QEvent::Resize) {
        repositionSendBtn();
    }
    if (watched == ui->chat_data_wid && event->type() == QEvent::MouseButtonPress) {
        ui->search_edit->clearFocus();
    }
    // 继续交给父类处理
    return QDialog::eventFilter(watched, event);
}

void ChatDialog::repositionSendBtn()
{
    if (!sendBtn) return;

    const int padding = 22; // 想更贴边就 0~4

    auto te = ui->textEdit;

    // contentsRect去掉边框(frame)后的内部区域
    const QRect r = te->contentsRect();

    const int x = r.right()  - sendBtn->width()  - padding + 1;
    const int y = r.bottom() - sendBtn->height() - padding + 10;

    sendBtn->move(x, y);
}

void ChatDialog::onSendClicked()
{
    const QString text = ui->textEdit->toPlainText().trimmed();
    if (text.isEmpty()){
        return;
    }

    //发送逻辑

    ui->textEdit->clear();
}

void ChatDialog::addChatUserList()
{
    // 创建QListWidgetItem，并设置自定义的widget
    for(int i = 0; i < 13; i++){
        int randomValue = QRandomGenerator::global()->bounded(100); // 生成0到99之间的随机整数
        int str_i = randomValue%strs.size();
        int head_i = randomValue%heads.size();
        int name_i = randomValue%names.size();

        auto *chat_user_wid = new ChatUserWid();
        chat_user_wid->SetInfo(names[name_i], heads[head_i], strs[str_i]);
        QListWidgetItem *item = new QListWidgetItem;//创建qt默认item
        //qDebug()<<"chat_user_wid sizeHint is " << chat_user_wid->sizeHint();
        item->setSizeHint(chat_user_wid->sizeHint());
        ui->chat_user_list->addItem(item);//添加默认item
        ui->chat_user_list->setItemWidget(item, chat_user_wid);//把item替换成自设
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

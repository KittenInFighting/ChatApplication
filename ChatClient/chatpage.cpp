#include "chatpage.h"
#include "ui_chatpage.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include <QEvent>
#include <QImageReader>
ChatPage::ChatPage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::ChatPage)
{
    ui->setupUi(this);

    ui->emoji_btn->setFixedSize(33, 33);
    ui->emoji_btn->setIcon(QIcon(":/res/emoji.png"));
    ui->emoji_btn->setIconSize(QSize(33, 33));
    ui->emoji_btn->setFlat(true);              // 去掉按钮边框
    ui->emoji_btn->setStyleSheet("border:none; padding:0px;"); // 无边框

    ui->file_btn->setFixedSize(33, 33);
    ui->file_btn->setIcon(QIcon(":/res/file.png"));
    ui->file_btn->setIconSize(QSize(33, 33));
    ui->file_btn->setFlat(true);
    ui->file_btn->setStyleSheet("border:none; padding:0px;");

    auto enableBg = [](QWidget* w){//允许背景由 QSS 来绘制/生效
        if (!w) return;
        w->setAttribute(Qt::WA_StyledBackground, true);
    };

    enableBg(ui->chat_data_wid);//右侧聊天区域
    enableBg(ui->mode_wid);

    ui->title_wid->setAttribute(Qt::WA_StyledBackground, true);
    ui->title_wid->setAttribute(Qt::WA_StyleSheetTarget, true);
    ui->mode_wid->setAttribute(Qt::WA_StyledBackground, true);

    ui->title_lab->setStyleSheet(R"(
    QLabel#title_lab {
        font-size: 15px;
        font-weight: bold;
    }
    )");
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

    ui->emoji_btn->setStyleSheet(R"(
    QPushButton{
    background: transparent;
    border: none;
    border-radius: 6px;
    padding: 2px;
    }
    QPushButton:hover{
     background-color: rgba(0,0,0,25);
    }
    QPushButton:pressed{
     background-color: rgba(0,0,0,45);
    }
    )");
    ui->file_btn->setStyleSheet(ui->emoji_btn->styleSheet());

    sendBtn->setFixedSize(100, 33);
    //显示在最上层
    sendBtn->raise();
    // 右下角留空隙
    const int padding = 15;
    te->setOverlayMargins(sendBtn->width() + padding + 10, sendBtn->height() + padding);
    //初次定位按钮
    repositionSendBtn();
    //textEdit 变化大小时，重新定位按钮
    connect(te, &MessageTextEdit::resized, this, &ChatPage::repositionSendBtn);
    auto updateSendBtnState = [=]() {
        const bool hasContent = !te->toPlainText().trimmed().isEmpty();
        sendBtn->setEnabled(hasContent);
    };
    updateSendBtnState();


    // 输入变化时实时更新
    connect(te, &QTextEdit::textChanged, this, updateSendBtnState);

    // 输入变化时实时更新
    connect(te, &QTextEdit::textChanged, this, updateSendBtnState);
    //点击发送
    connect(sendBtn, &QPushButton::clicked, this, &ChatPage::onSendClicked);
    installEventFilter(this);
}

ChatPage::~ChatPage()
{
    delete ui;
}
void ChatPage::repositionSendBtn()
{
    if (!sendBtn) return;

    const int padding = 22;

    auto te = ui->textEdit;

    // contentsRect去掉边框(frame)后的内部区域
    const QRect r = te->contentsRect();

    const int x = r.right()  - sendBtn->width()  - padding + 1;
    const int y = r.bottom() - sendBtn->height() - padding + 10;

    sendBtn->move(x, y);
}

void ChatPage::onSendClicked()
{
    const QString text = ui->textEdit->toPlainText().trimmed();
    if (text.isEmpty()){
        return;
    }

    //发送逻辑
    auto pTextEdit = ui->textEdit;
    ChatRole role = ChatRole::Self;
    QString userName = QStringLiteral("牡牡蛎");
    QString userIcon = ":/res/1.jpg";

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    for(int i=0; i<msgList.size(); ++i)
    {
        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;
        if(type == "text")
        {
            pBubble = new TextBubble(role, msgList[i].content);
        }
        else if(type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->text_wid->appendChatItem(pChatItem);
        }
    }
    ui->textEdit->clear();
}
bool ChatPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->textEdit && event->type() == QEvent::Resize) {
        repositionSendBtn();
    }
    // 继续交给父类处理
    return QWidget::eventFilter(watched, event);
}

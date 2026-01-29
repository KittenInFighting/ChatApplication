#include "chatpage.h"
#include "ui_chatpage.h"
#include "chatitembase.h"
#include "textbubble.h"
#include "picturebubble.h"
#include "usermgr.h"
#include "tcpmgr.h"
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
    // const QString text = ui->textEdit->toPlainText().trimmed();
    // if (text.isEmpty()){
    //     return;
    // }

    // //发送逻辑
    // auto pTextEdit = ui->textEdit;
    // ChatRole role = ChatRole::Self;
    // QString userName = QStringLiteral("牡牡蛎");
    // QString userIcon = ":/res/1.jpg";

    // const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    // for(int i=0; i<msgList.size(); ++i)
    // {
    //     QString type = msgList[i].msgFlag;
    //     ChatItemBase *pChatItem = new ChatItemBase(role);
    //     pChatItem->setUserName(userName);
    //     pChatItem->setUserIcon(QPixmap(userIcon));
    //     QWidget *pBubble = nullptr;
    //     if(type == "text")
    //     {
    //         pBubble = new TextBubble(role, msgList[i].content);
    //     }
    //     else if(type == "image")
    //     {
    //         pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
    //     }
    //     else if(type == "file")
    //     {

    //     }
    //     if(pBubble != nullptr)
    //     {
    //         pChatItem->setWidget(pBubble);
    //         ui->text_wid->appendChatItem(pChatItem);
    //     }
    // }
    // ui->textEdit->clear();

    if (_user_info == nullptr) {
        qDebug() << "friend_info is empty";
        return;
    }

    auto user_info = UserMgr::GetInstance()->GetUserInfo();
    auto pTextEdit = ui->textEdit;
    ChatRole role = ChatRole::Self;
    QString userName = user_info->_name;
    QString userIcon = user_info->_icon;

    const QVector<MsgInfo>& msgList = pTextEdit->getMsgList();
    QJsonObject textObj;
    QJsonArray textArray;
    int txt_size = 0;

    for(int i=0; i<msgList.size(); ++i)
    {
        //消息内容长度不合规就跳过
        if(msgList[i].content.length() > 1024){
            continue;
        }

        QString type = msgList[i].msgFlag;
        ChatItemBase *pChatItem = new ChatItemBase(role);
        pChatItem->setUserName(userName);
        pChatItem->setUserIcon(QPixmap(userIcon));
        QWidget *pBubble = nullptr;

        if(type == "text")
        {
            //生成唯一id
            QUuid uuid = QUuid::createUuid();
            //转为字符串
            QString uuidString = uuid.toString();

            pBubble = new TextBubble(role, msgList[i].content);
            if(txt_size + msgList[i].content.length()> 1024){
                textObj["fromuid"] = user_info->_uid;
                textObj["touid"] = _user_info->_uid;
                textObj["text_array"] = textArray;
                QJsonDocument doc(textObj);
                QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
                //发送并清空之前累计的文本列表
                txt_size = 0;
                textArray = QJsonArray();
                textObj = QJsonObject();
                //发送tcp请求给chat server
                emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
            }

            //将bubble和uid绑定，以后可以等网络返回消息后设置是否送达
            //_bubble_map[uuidString] = pBubble;
            txt_size += msgList[i].content.length();
            QJsonObject obj;
            QByteArray utf8Message = msgList[i].content.toUtf8();
            obj["content"] = QString::fromUtf8(utf8Message);
            obj["msgid"] = uuidString;
            textArray.append(obj);
            auto txt_msg = std::make_shared<TextChatData>(uuidString, obj["content"].toString(),
                                                          user_info->_uid, _user_info->_uid);
            emit sig_append_send_chat_msg(txt_msg);
        }
        else if(type == "image")
        {
            pBubble = new PictureBubble(QPixmap(msgList[i].content) , role);
        }
        else if(type == "file")
        {

        }
        //发送消息
        if(pBubble != nullptr)
        {
            pChatItem->setWidget(pBubble);
            ui->text_wid->appendChatItem(pChatItem);
        }
    }
    qDebug() << "textArray is " << textArray ;
    //发送给服务器
    textObj["text_array"] = textArray;
    textObj["fromuid"] = user_info->_uid;
    textObj["touid"] = _user_info->_uid;
    QJsonDocument doc(textObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);
    //发送并清空之前累计的文本列表
    txt_size = 0;
    textArray = QJsonArray();
    textObj = QJsonObject();
    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_TEXT_CHAT_MSG_REQ, jsonData);
}
bool ChatPage::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == ui->textEdit && event->type() == QEvent::Resize) {
        repositionSendBtn();
    }
    // 继续交给父类处理
    return QWidget::eventFilter(watched, event);
}
void ChatPage::AppendChatMsg(std::shared_ptr<TextChatData> msg)
{
    auto self_info = UserMgr::GetInstance()->GetUserInfo();
    ChatRole role;
    //添加聊天显示!
    if (msg->_from_uid == self_info->_uid) {
        role = ChatRole::Self;
        ChatItemBase* pChatItem = new ChatItemBase(role);

        pChatItem->setUserName(self_info->_name);
        pChatItem->setUserIcon(QPixmap(self_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->text_wid->appendChatItem(pChatItem);
    }
    else {
        role = ChatRole::Other;
        ChatItemBase* pChatItem = new ChatItemBase(role);
        auto friend_info = UserMgr::GetInstance()->GetFriendById(msg->_from_uid);
        if (friend_info == nullptr) {
            return;
        }
        pChatItem->setUserName(friend_info->_name);
        pChatItem->setUserIcon(QPixmap(friend_info->_icon));
        QWidget* pBubble = nullptr;
        pBubble = new TextBubble(role, msg->_msg_content);
        pChatItem->setWidget(pBubble);
        ui->text_wid->appendChatItem(pChatItem);
    }
}

void ChatPage::SetUserInfo(std::shared_ptr<UserInfo> user_info)
{
    _user_info = user_info;
    //设置ui界面
    ui->title_lab->setText(_user_info->_name);
    ui->text_wid->removeAllItem();
    for(auto & msg : user_info->_chat_msgs){
        AppendChatMsg(msg);
    }
}

void ChatPage::clearItems()
{
    ui->text_wid->removeAllItem();
}

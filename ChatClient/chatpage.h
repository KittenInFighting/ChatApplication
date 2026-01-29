#ifndef CHATPAGE_H
#define CHATPAGE_H

#include "userdata.h"
#include <QWidget>
#include <QPushButton>

namespace Ui {
class ChatPage;
}

class ChatPage : public QWidget
{
    Q_OBJECT

public:
    explicit ChatPage(QWidget *parent = nullptr);
    ~ChatPage();
    void AppendChatMsg(std::shared_ptr<TextChatData> msg);
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void repositionSendBtn();//重新定位发送按钮
    void onSendClicked();//发送按钮被点击
    void clearItems();
    Ui::ChatPage *ui;
    QPushButton *sendBtn = nullptr;
    std::shared_ptr<UserInfo> _user_info;
     QMap<QString, QWidget*>  _bubble_map;
signals:
    void sig_append_send_chat_msg(std::shared_ptr<TextChatData> msg);
};

#endif // CHATPAGE_H

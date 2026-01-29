#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include "listitembase.h"
#include "userdata.h"
#include <QWidget>

namespace Ui {
class ChatUserWid;
}

class ChatUserWid : public ListItemBase
{
    Q_OBJECT
public:
    explicit ChatUserWid(QWidget *parent = nullptr);
    ~ChatUserWid();

    QSize sizeHint() const override {
        return QSize(200, 60); // 返回自定义的尺寸
    }

    void SetInfo(QString name, QString head, QString msg, QString time);
    void SetInfo(std::shared_ptr<UserInfo> user_info);
    void SetInfo(std::shared_ptr<FriendInfo> friend_info);
    std::shared_ptr<UserInfo> GetUserInfo();
    void updateLastMsg(std::vector<std::shared_ptr<TextChatData>> msgs);
protected:
    void resizeEvent(QResizeEvent* e) override;
private:
    void updateNameElide();
    void updateMsgElide();
    Ui::ChatUserWid *ui;
    QString _name;
    QString _head;
    QString _msg;
    QString _time;
    std::shared_ptr<UserInfo> _user_info;
};

#endif // CHATUSERWID_H

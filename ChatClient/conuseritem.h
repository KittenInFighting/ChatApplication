#ifndef CONUSERITEM_H
#define CONUSERITEM_H

#include "listitembase.h"
#include "userdata.h"
#include <QWidget>

namespace Ui {
class ConUserItem;
}

class ConUserItem : public ListItemBase
{
    Q_OBJECT
public:
    explicit ConUserItem(QWidget *parent = nullptr);
    ~ConUserItem();

    QSize sizeHint() const override {
        return QSize(200, 60); // 返回自定义的尺寸
    }
    void SetInfo(std::shared_ptr<AuthInfo> auth_info);
    void SetInfo(std::shared_ptr<AuthRsp> auth_rsp);
    void SetInfo(int uid, QString name, QString head);
    std::shared_ptr<UserInfo> GetInfo();
protected:
    void resizeEvent(QResizeEvent* e) override;
private:
    void updateNameElide();
    void updateMsgElide();
    Ui::ConUserItem *ui;
    QString _msg = "[在线]";
    std::shared_ptr<UserInfo> _info;
};

#endif // CONUSERITEM_H

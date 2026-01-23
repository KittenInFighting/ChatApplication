#ifndef AUTHENFRIEND_H
#define AUTHENFRIEND_H

#include "userdata.h"
#include <QDialog>

namespace Ui {
class AuthenFriend;
}

class AuthenFriend : public QDialog
{
    Q_OBJECT
public:
    explicit AuthenFriend(QWidget *parent = nullptr);
    ~AuthenFriend();
    bool eventFilter(QObject *obj, QEvent *event);
    void SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info);
private:
    Ui::AuthenFriend *ui;
    std::shared_ptr<ApplyInfo> _apply_info;
    //处理确认回调
private slots:
    void SlotAuthenSure();
    //处理取消回调
    void SlotAuthenCancel();
};

#endif // AUTHENFRIEND_H

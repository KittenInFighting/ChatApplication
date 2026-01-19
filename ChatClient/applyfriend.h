#ifndef APPLYFRIEND_H
#define APPLYFRIEND_H

#include "userdata.h"
#include <QDialog>

namespace Ui {
class ApplyFriend;
}

class ApplyFriend : public QDialog
{
    Q_OBJECT

public:
    explicit ApplyFriend(QWidget *parent = nullptr);
    ~ApplyFriend();
    bool eventFilter(QObject *obj, QEvent *event);
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);
private:
    Ui::ApplyFriend *ui;
    std::shared_ptr<SearchInfo> _si;
    //处理确认回调
    void SlotApplySure();
    //处理取消回调
    void SlotApplyCancel();
};

#endif // APPLYFRIEND_H

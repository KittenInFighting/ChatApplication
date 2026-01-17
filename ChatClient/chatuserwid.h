#ifndef CHATUSERWID_H
#define CHATUSERWID_H

#include "listitembase.h"
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
        return QSize(190, 60); // 返回自定义的尺寸
    }

    void SetInfo(QString name, QString head, QString msg, QString time);
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
};

#endif // CHATUSERWID_H

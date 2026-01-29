#ifndef GROUPTIPITEM_H
#define GROUPTIPITEM_H

#include "listitembase.h"
#include <QWidget>

namespace Ui {
class GroupTipItem;
}

class GroupTipItem : public ListItemBase
{
    Q_OBJECT

public:
    explicit GroupTipItem(QWidget *parent = nullptr);
    ~GroupTipItem();
    QSize sizeHint() const override;
    void SetGroupTip(QString str);
private slots:
    void on_pushButton_clicked();

private:
    QString _tip;
    Ui::GroupTipItem *ui;
signals:
    void sig_switch_apply_friend_page();
};

#endif // GROUPTIPITEM_H

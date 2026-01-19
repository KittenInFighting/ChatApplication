#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "global.h"
#include "statewidget.h"
#include <QDialog>
#include <QPushButton>

namespace Ui {
class ChatDialog;
}

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void CloseFindDlg();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void addChatUserList();//用于测试
    void slot_side_chat();
    void slot_side_contact();
    void on_add_btn_clicked();

private:
    void ShowList(bool bsearch);
    void ClearLabelState(StateWidget *lb);
    void AddLBGroup(StateWidget *lb);
    void ShowSearch();
    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget*> _lb_list;
    std::shared_ptr<QDialog> _find_dlg;
};

#endif // CHATDIALOG_H

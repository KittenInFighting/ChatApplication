#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "global.h"
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

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    void onSendClicked();//发送按钮被点击
    void addChatUserList();//用于测试
private:
    void ShowList(bool bsearch);
    void repositionSendBtn();//重新定位发送按钮
    Ui::ChatDialog *ui;
    QPushButton *sendBtn = nullptr;

    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
};

#endif // CHATDIALOG_H

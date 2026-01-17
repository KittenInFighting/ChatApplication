#ifndef CHATPAGE_H
#define CHATPAGE_H

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
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
private:
    void repositionSendBtn();//重新定位发送按钮
    void onSendClicked();//发送按钮被点击
    Ui::ChatPage *ui;
    QPushButton *sendBtn = nullptr;
};

#endif // CHATPAGE_H

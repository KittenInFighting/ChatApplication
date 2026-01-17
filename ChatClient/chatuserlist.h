#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include <QPropertyAnimation>
class ChatUserList: public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);
    void addChatUserWidget(QWidget *w);   //ChatUserWid 进来
    void refreshScrollRange();
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void smoothScrollTo(int target);
private:
    void syncHoverUnderCursor();
    QPropertyAnimation *m_scrollAnim = nullptr;
    int m_targetValue = 0;
signals:
    void sig_loading_chat_user();
};


#endif // CHATUSERLIST_H

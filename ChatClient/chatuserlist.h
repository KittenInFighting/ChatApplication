#ifndef CHATUSERLIST_H
#define CHATUSERLIST_H

#include <QListWidget>
#include <QWheelEvent>
#include <QEvent>
#include <QScrollBar>
#include <QDebug>
#include <QPropertyAnimation>
class QMouseEvent;
class ChatUserList: public QListWidget
{
    Q_OBJECT
public:
    ChatUserList(QWidget *parent = nullptr);
    void addChatUserWidget(QWidget *w);   //ChatUserWid 进来
    void refreshScrollRange();
    void updateHoverItemAt(const QPoint& viewportPos);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void smoothScrollTo(int target);
    void scrollContentsBy(int dx, int dy) override;

private:
    void updateHoverItem();
    QListWidgetItem* m_pressedItem = nullptr;
    bool m_pressedItemWasSelected = false;
    void clearHoverItem();
    QPropertyAnimation *m_scrollAnim = nullptr;
    QListWidgetItem* m_hoverItem = nullptr;
    int m_targetValue = 0;
    QPoint m_lastVpPos;
    bool m_hasLastVpPos = false;
signals:
    void sig_loading_chat_user();
};


#endif // CHATUSERLIST_H

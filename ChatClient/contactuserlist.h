#ifndef CONTACTUSERLIST_H
#define CONTACTUSERLIST_H

#include "userdata.h"
#include <QListWidget>
#include <QEvent>
#include <QWheelEvent>
#include <QScrollBar>
#include <QDebug>
#include <memory>
#include <QPropertyAnimation>

class ConUserItem;

class ContactUserList : public QListWidget
{
    Q_OBJECT
public:
    ContactUserList(QWidget *parent = nullptr);
    void ShowRedPoint(bool bshow = true);//有申请 显示红点
    void addContactUserWidget(QWidget *w);   //ContactUserWid 进来
    void refreshScrollRange();
    void updateHoverItemAt(const QPoint& viewportPos);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override ;
    void smoothScrollTo(int target);
    void scrollContentsBy(int dx, int dy) override;
private:
    void addContactUserList();

    void updateHoverItem();
    QPropertyAnimation *m_scrollAnim = nullptr;
    QListWidgetItem* m_hoverItem = nullptr;
    int m_targetValue = 0;
    QPoint m_lastVpPos;
    bool m_hasLastVpPos = false;

public slots:
    void slot_item_clicked(QListWidgetItem *item);
    //     void slot_add_auth_firend(std::shared_ptr<AuthInfo>);
    //     void slot_auth_rsp(std::shared_ptr<AuthRsp>);
signals:
    void sig_loading_contact_user();
    void sig_switch_friend_info_page(std::shared_ptr<UserInfo> user_info);
private:
    ConUserItem* _add_friend_item;
    QListWidgetItem * _groupitem;
};

#endif // CONTACTUSERLIST_H

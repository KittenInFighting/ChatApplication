#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QDialog>
#include <QPoint>
#include <QPushButton>
#include <QListWidgetItem>

namespace Ui {
class ChatDialog;
}

class QLabel;
class QEvent;
class QMouseEvent;
class QPaintEvent;
class QResizeEvent;
class QToolButton;

class ChatDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ChatDialog(QWidget *parent = nullptr);
    ~ChatDialog();
    void CloseFindDlg();
    void RefreshApplyRedPoint();
    void UpdateChatMsg(std::vector<std::shared_ptr<TextChatData>> msgdata);
protected:
    bool eventFilter(QObject *watched, QEvent *event) override;
    void paintEvent(QPaintEvent *event) override;
    void resizeEvent(QResizeEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void mouseDoubleClickEvent(QMouseEvent *event) override;
    void changeEvent(QEvent *event) override;
    bool nativeEvent(const QByteArray &eventType, void *message, qintptr *result) override;

private slots:
    void addChatUserList();//用于测试
    void slot_side_chat();
    void slot_side_contact();
    void on_add_btn_clicked();
    void slot_user_search(std::shared_ptr<SearchInfo> si);
    void slot_find_failed();
    void slot_friend_apply(std::shared_ptr<AddFriendApply> apply);
    void slot_auth_rsp(std::shared_ptr<AuthRsp> auth_rsp);
    void slot_add_auth_friend(std::shared_ptr<AuthInfo> auth_info);
    void slot_jump_chat_item(std::shared_ptr<SearchInfo> si);
    void slot_loading_chat_user();//加载聊天列表
    void slot_loading_contact_user();//加载好友列表
    void slot_friend_info_page(std::shared_ptr<UserInfo> user_info);//好友信息界面
    void slot_switch_apply_friend_page();
    void slot_jump_chat_item_from_infopage(std::shared_ptr<UserInfo> user_info);
    void slot_chat_item_clicked(QListWidgetItem *item);
    void slot_chat_item_cancel();
    void slot_text_chat_msg(std::shared_ptr<TextChatMsg> msg);
    void slot_append_send_chat_msg(std::shared_ptr<TextChatData> msgdata);
private:
    void initTitleBar();
    void updateWindowMask();
    void updateTitleButtons();
    void toggleMaxRestore();
    bool hitTitleBar(const QPoint &pos) const;
    void ShowList(bool bsearch);
    void ClearLabelState(StateWidget *lb);
    void AddLBGroup(StateWidget *lb);
    void ShowSearch();
    void SetSelectChatItem(int uid = 0);
    void SetSelectChatPage(int uid = 0);
    void loadMoreChatUser();//加载聊天列表
    void loadMoreConUser();//加载联系人列表
    QMap<int, QListWidgetItem*> _chat_items_added;
    QWidget *m_titleBar = nullptr;
    QLabel *m_titleLabel = nullptr;
    QToolButton *m_minBtn = nullptr;
    QToolButton *m_maxBtn = nullptr;
    QToolButton *m_closeBtn = nullptr;
    bool m_dragging = false;
    QPoint m_dragPos;

    Ui::ChatDialog *ui;
    ChatUIMode _mode;
    ChatUIMode _state;
    bool _b_loading;
    QList<StateWidget*> _lb_list;
    std::shared_ptr<QDialog> _find_dlg;
    int _cur_chat_uid;
    bool _has_chat_context = false;
    QWidget* _last_widget;
signals:
    void sig_jump_chat_item(std::shared_ptr<SearchInfo> si);
};

#endif // CHATDIALOG_H

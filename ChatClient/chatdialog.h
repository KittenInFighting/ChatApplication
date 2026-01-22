#ifndef CHATDIALOG_H
#define CHATDIALOG_H

#include "global.h"
#include "statewidget.h"
#include "userdata.h"
#include <QDialog>
#include <QPoint>
#include <QPushButton>

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
};

#endif // CHATDIALOG_H

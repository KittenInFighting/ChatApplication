#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include "clickablelabel.h"
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = nullptr);
    ~LoginDialog();
    void setAvatar(const QString &path);
protected:
    void resizeEvent(QResizeEvent *event) override;
    void closeEvent(QCloseEvent *event) override;//重写close事件
private:
    Ui::LoginDialog *ui;
    QPixmap _avatarSrc;
    QPoint m_dragPos;
    void updateAvatar(); // 根据当前 QLabel 大小重新裁圆并显示
    static QPixmap makeRoundPixmap(const QPixmap &src, int diameter,int borderWidth);
    void mouseMoveEvent(QMouseEvent *e)override;
    void mousePressEvent(QMouseEvent *e)override;
signals:
    void switchRegister();//用于激活注册页面
    void sigLoginClosed();//用于发送关闭信号
    void switchReset();
public slots:
    void slot_forget_pwd();
};

#endif // LOGINDIALOG_H

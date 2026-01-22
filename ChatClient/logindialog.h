#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include "global.h"
#include <QDialog>
#include "clickablelabel.h"
#include <QPixmap>
#include <QPoint>
#include <QMouseEvent>
#include <QGraphicsDropShadowEffect>
#include <QPainter>

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
    int _uid;//用于登录回包
    QString _token;//用于登录回包
    bool m_dragging = false;//控制是否能拖动
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    void updateAvatar(); // 根据当前 QLabel 大小重新裁圆并显示
    static QPixmap makeRoundPixmap(const QPixmap &src, int diameter,int borderWidth);
    bool checkCount();//初步检测账号
    bool checkPwd();//初步检测密码
    void initHttpHandlers();//处理http回包
    void showLoginErrorDialog();
    bool hasBadRepeatOrSequence(const QString& s) const;
    void mouseMoveEvent(QMouseEvent *e)override;
    void mousePressEvent(QMouseEvent *e)override;
    void mouseReleaseEvent(QMouseEvent *e) override;
signals:
    void switchRegister();//用于激活注册页面
    void sigLoginClosed();//用于发送关闭信号
    void switchReset();//密码重置界面
    void switchchat();//切换聊天界面
    void sig_connect_tcp(ServerInfo);//tcp连接
public slots:
    void slot_forget_pwd();//发送信号给mainwindow切换界面
    void slot_login_mod_finish(ReqId id, QString res, ErrorCodes err);//处理短链接的login_mod
    void slot_tcp_con_finish(bool bsuccess);//长连接成功开始处理
    void slot_login_failed();
private slots:
    void on_signIn_pushButton_clicked();
};

class RoundedMask : public QWidget {
public:
    explicit RoundedMask(QWidget* parent, int radius)
        : QWidget(parent), radius_(radius) {
        setAttribute(Qt::WA_TranslucentBackground, true);
    }

protected:
    void paintEvent(QPaintEvent*) override {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, true);
        p.setPen(Qt::NoPen);
        p.setBrush(QColor(0, 0, 0, 120)); // 半透明黑
        p.drawRoundedRect(rect().adjusted(0, 0, -1, -1), radius_, radius_);
    }

private:
    int radius_;
};
#endif // LOGINDIALOG_H

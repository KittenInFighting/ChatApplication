/******************************************************************************
 *
 * @file       mainwindow.h
 * @brief      MainWindow
 *
 * @author     KittenInFight
 * @date       2025/4/20
 *
 *****************************************************************************/

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include "logindialog.h"
#include "registerdialog.h"
#include "resetdialog.h"
#include "chatdialog.h"

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void SlotSwitchReg();//进入注册界面
    void SlotSwtichLogin();//注册界面返回登录界面
    void SlotSwtichLogin2();//忘记密码界面返回登录界面
    void SlotSwitchReset();//进入忘记密码界面
    void SlotSwichChat();//登录成功切换界面
private:
    Ui::MainWindow *ui;
    QStackedWidget *_stack = nullptr;//管理登录失败的弹窗背景阴影
    LoginDialog *_login_dlg = nullptr;//管理登录界面
    RegisterDialog *_reg_dlg = nullptr;//管理注册界面
    ResetDialog* _reset_dlg = nullptr;//管理忘记密码界面
    ChatDialog* _chat_dlg = nullptr;

    void adaptToPage(QWidget *page);
};
#endif // MAINWINDOW_H

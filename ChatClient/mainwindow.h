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
    void SlotSwitchReg();
    void SlotSwtichLogin();
    void SlotSwtichLogin2();
    void SlotSwitchReset();
private:
    Ui::MainWindow *ui;
    QStackedWidget *_stack = nullptr;
    LoginDialog *_login_dlg = nullptr;
    RegisterDialog *_reg_dlg = nullptr;
    ResetDialog* _reset_dlg = nullptr;

    void adaptToPage(QWidget *page);
};
#endif // MAINWINDOW_H

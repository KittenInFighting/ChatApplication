#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "tcpmgr.h"
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    //创建登录界面
    _login_dlg = new LoginDialog();
    _login_dlg->setAttribute(Qt::WA_DeleteOnClose); // 关闭即析构
    connect(_login_dlg, &QObject::destroyed, this, [this]() {
        _login_dlg = nullptr; // 防止悬空指针
    });

    _login_dlg -> adjustSize();
    _login_dlg -> show();

    //connect Login窗口的处理
    //注册
    connect(_login_dlg, &LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);
    //忘记密码
    connect(_login_dlg, &LoginDialog::switchReset, this, &MainWindow::SlotSwitchReset);

    //如果登录界面关闭，则注册页面也关闭
    connect(_login_dlg, &LoginDialog::sigLoginClosed, this, [this]() {
        if (_reg_dlg) {
            _reg_dlg->close();
        }
    });

    //登录确认连接tcp服务器，切换界面
    connect(_login_dlg, &LoginDialog::switchchat, this, &MainWindow::SlotSwichChat);

    //处理聊天窗口的服务器断联
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_tcp_disconnect, this, &MainWindow::SlotTcpDisconnect);
    //调试用
    //emit _login_dlg -> switchchat();
}

void MainWindow::adaptToPage(QWidget *page)
{

}

void MainWindow::SlotSwichChat()
{
    QMessageBox::information(this, tr("提示"), tr("聊天服务连接成功，正在切换聊天界面"));
    if(_login_dlg){
        _login_dlg->close(); // 触发析构
    }
    if (!_chat_dlg) {
        _chat_dlg = new ChatDialog();
        _chat_dlg->setAttribute(Qt::WA_DeleteOnClose); // 聊天界面也关闭就析构,聊天界面暂时这样考虑，后期可以修改成只是隐藏
        connect(_chat_dlg, &QObject::destroyed, this, [this]() {
            _chat_dlg = nullptr; // 避免悬空指针
        });
    }
    _chat_dlg -> adjustSize();
    _chat_dlg -> show();
    _chat_dlg->RefreshApplyRedPoint();

}

void MainWindow::SlotTcpDisconnect()
{
    QMessageBox::warning(this, tr("网络连接断开"), tr("请检查网络稍后重试"));
    if(_chat_dlg){
        _chat_dlg->close(); // 触发析构
    }
    if (!_login_dlg) {//返回登录界面
        _login_dlg = new LoginDialog();
        _login_dlg->setAttribute(Qt::WA_DeleteOnClose); //
        connect(_login_dlg, &QObject::destroyed, this, [this]() {
            _login_dlg = nullptr; // 避免悬空指针
        });
    }
    _login_dlg -> adjustSize();
    _login_dlg -> show();
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::SlotSwitchReg()
{

    if (!_reg_dlg) {
        _reg_dlg = new RegisterDialog();
        _reg_dlg->setAttribute(Qt::WA_DeleteOnClose); // 关闭就析构
        connect(_reg_dlg, &QObject::destroyed, this, [this]() {
            _reg_dlg = nullptr; // 避免悬空指针
        });
        //连接注册界面返回登录界面信号和槽函数
        connect(_reg_dlg, &RegisterDialog::sigSwitchLogin, this, &MainWindow::SlotSwtichLogin);
    }

    // //把注册界面移动到登录界面正中央
    // _reg_dlg -> move(_login_dlg -> geometry().center() - _reg_dlg -> rect().center());
    //_login_dlg -> hide();

    _reg_dlg -> adjustSize();
     QScreen *screen = QGuiApplication::screenAt(QCursor::pos()); // 获取鼠标所在屏幕
     if (!screen) { // 如果没取到屏幕
         screen = QGuiApplication::primaryScreen(); // 使用主屏
     }
     if (screen) { // 确保屏幕有效
         QRect screenRect = screen->availableGeometry(); // 屏幕可用区域（不含任务栏）
         QPoint center = screenRect.center(); // 屏幕中心点
         QRect dlgRect(QPoint(0, 0), _reg_dlg->size()); // 对话框矩形（左上角在原点）
         _reg_dlg->move(center - dlgRect.center()); // 移动到屏幕中央
     }
    _reg_dlg -> show();
}

void MainWindow::SlotSwitchReset()
{
    //创建一个CentralWidget
    if (!_reset_dlg) {
        _reset_dlg = new ResetDialog();
        _reset_dlg->setAttribute(Qt::WA_DeleteOnClose);//关闭就析构
        connect(_reset_dlg, &QObject::destroyed, this, [this]() {
            _reset_dlg = nullptr; // 避免悬空指针
        });
        //连接注册 返回登录信号和槽函数
        connect(_reset_dlg, &ResetDialog::switchLogin2, this, &::MainWindow::SlotSwtichLogin2);
    }
    _reset_dlg -> adjustSize();
    QScreen *screen = QGuiApplication::screenAt(QCursor::pos()); // 获取鼠标所在屏幕
    if (!screen) { // 如果没取到屏幕
        screen = QGuiApplication::primaryScreen(); // 使用主屏
    }
    if (screen) { // 确保屏幕有效
        QRect screenRect = screen->availableGeometry(); // 屏幕可用区域
        QPoint center = screenRect.center(); // 屏幕中心点
        QRect dlgRect(QPoint(0, 0), _reset_dlg->size()); // 对话框矩形
        _reset_dlg->move(center - dlgRect.center()); // 移动到屏幕中央
    }
    _reset_dlg -> show();
}
//
void MainWindow::SlotSwtichLogin()
{
    if (_reg_dlg) {
        _reg_dlg->close(); // 触发析构
    }
}

//从重置界面返回登录界面
void MainWindow::SlotSwtichLogin2()
{
    if (_reset_dlg) {
        _reset_dlg->close(); // 触发析构
    }
}



#include "mainwindow.h"
#include "ui_mainwindow.h"

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

    //connect Login窗口的信息
    connect(_login_dlg, &LoginDialog::switchRegister,this,&MainWindow::SlotSwitchReg);
    //如果登录界面关闭，则注册页面也关闭
    connect(_login_dlg, &LoginDialog::sigLoginClosed, this, [this]() {
        if (_reg_dlg) {
            _reg_dlg->close();
        }
    });
}

void MainWindow::adaptToPage(QWidget *page)
{

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
        //连接注册界面返回登录界面信号
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

void MainWindow::SlotSwtichLogin()
{
    if (_reg_dlg) {
        _reg_dlg->close(); // 触发析构
    }
}

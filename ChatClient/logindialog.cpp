#include "logindialog.h"
#include "ui_logindialog.h"

#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QDebug>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);

    //设置窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, true);

    //设置窗口右上方的closeBtn
    connect(ui->btnClose, &QToolButton::clicked, this, &LoginDialog::close);
    // 让头像 QLabel 固定成正方形
    ui->head_label->setFixedSize(80, 80);
    ui->head_label->setScaledContents(false);

    // 2) 设置一张头像
    setAvatar(":/res/1.jpg");

    // 让 label 不吃背景
    ui->head_label->setStyleSheet("background: transparent;");

    //geiregister_label绑定点击事件
    ui -> register_label -> setCursor(Qt::PointingHandCursor);
    connect(ui->register_label,&ClickableLabel::clicked,this,&LoginDialog::switchRegister);

    //设置LineEdit文字居中
    ui->count_lineEdit->setAlignment(Qt::AlignHCenter);
    ui->pwd_lineEdit->setAlignment(Qt::AlignHCenter);

    //设置密码不可见
    ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDlg" << "\n";
    delete ui;
}

void LoginDialog::setAvatar(const QString &path)
{
    _avatarSrc = QPixmap(path);
    updateAvatar();
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    // 如果你的 label头像 不是 fixedSize，而是会跟布局变化，这里能保证始终是圆的
    updateAvatar();
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    emit sigLoginClosed();
    QDialog::closeEvent(event);
}

void LoginDialog::updateAvatar()
{
    if (_avatarSrc.isNull() || !ui->head_label) return;

    int d = qMin(ui->head_label->width(), ui->head_label->height());
    if (d <= 0) return;

    QPixmap round = makeRoundPixmap(_avatarSrc, d,3);
    ui->head_label->setPixmap(round);
}

QPixmap LoginDialog::makeRoundPixmap(const QPixmap &src, int diameter,int borderWidth)
{
    if (src.isNull() || diameter <= 0) return QPixmap();

    // 先缩放到足够大，再裁剪，保证清晰
    QPixmap scaled = src.scaled(diameter, diameter,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);

    QPixmap dst(diameter, diameter);
    dst.fill(Qt::transparent);

    QPainter p(&dst);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 圆形裁剪
    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    p.setClipPath(path);

    // 居中绘制
    int x = (diameter - scaled.width()) / 2;
    int y = (diameter - scaled.height()) / 2;
    p.drawPixmap(x, y, scaled);

    // 画白色描边
    p.setClipping(false);
    QPen pen(QColor(255, 255, 255, 255), borderWidth);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // 让线条完整显示在图内
    const int half = borderWidth / 2;
    p.drawEllipse(half, half, diameter - borderWidth, diameter - borderWidth);
    return dst;
}

void LoginDialog::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
}

void LoginDialog::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton) {
        move(e->globalPosition().toPoint() - m_dragPos);
        e->accept();
    }
}



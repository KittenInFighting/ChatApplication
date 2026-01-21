#include "findsuccessdlg.h"
#include "ui_findsuccessdlg.h"
#include "applyfriend.h"
#include <QDir>
#include <QPainter>
#include <QPainterPath>

static QPixmap makeRoundPixmap(const QPixmap& src, int size)
{
    if (src.isNull()) return QPixmap();

    QPixmap scaled = src.scaled(size, size, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation);

    QPixmap out(size, size);
    out.fill(Qt::transparent);

    QPainter p(&out);
    p.setRenderHint(QPainter::Antialiasing, true);

    QPainterPath path;
    path.addEllipse(0, 0, size, size);
    p.setClipPath(path);
    p.drawPixmap(0, 0, scaled);

    return out;
}

FindSuccessDlg::FindSuccessDlg(QWidget *parent)
    : QDialog(parent),_parent(parent)
    ,ui(new Ui::FindSuccessDlg)
{
    ui->setupUi(this);
    //setWindowFlags(windowFlags() | Qt::FramelessWindowHint);

    //获取查找用户信息
    QString app_path = QCoreApplication::applicationDirPath();
    QString pix_path = QDir::toNativeSeparators(app_path +
                                                QDir::separator() + "static"+QDir::separator()+"head_1.jpg");

    //头像：裁圆 + 固定大小
    const int avatarSize = 50;
    ui->head_label->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(pix_path);
    ui->head_label->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->head_label->setScaledContents(false);

    // QPixmap head_pix(pix_path);
    // head_pix = head_pix.scaled(ui->head_label->size(),
    //                            Qt::KeepAspectRatio, Qt::SmoothTransformation);
    // ui->head_label->setPixmap(head_pix);
    //ui->add_friend_btn->SetState("normal","hover","press");

    this->setModal(true);

}

FindSuccessDlg::~FindSuccessDlg()
{
    qDebug()<<"FindSuccessDlg destruct";
    delete ui;
}

void FindSuccessDlg::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    ui->name_label->setText(si->_name);
    _si = si;
}

void FindSuccessDlg::on_add_friend_btn_clicked()
{
    //添加好友界面弹出
    this -> hide();//后期考虑优化

    //弹出好友添加界面
    auto applyFriend = new ApplyFriend(_parent);
    applyFriend->SetSearchInfo(_si);
    applyFriend->setModal(true);
    applyFriend->show();
}


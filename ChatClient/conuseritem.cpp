#include "conuseritem.h"
#include "ui_conuseritem.h"

#include <QPainter>
#include <QPainterPath>
#include <QFontMetrics>
#include <QDateTime>
#include <QFontMetrics>
#include <QResizeEvent>

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
void ConUserItem::updateMsgElide()
{
    // 只显示一行，超出显示省略号
    ui->user_state_lab->setWordWrap(false);

    // 当前 label 的“可用宽度”
    int maxW = ui->user_state_lab->contentsRect().width();

    QFontMetrics fm(ui->user_state_lab->font());
    ui->user_state_lab->setText(fm.elidedText(_msg, Qt::ElideRight, maxW));
}

void ConUserItem::updateNameElide()
{
    ui->user_name_lab->setWordWrap(false);

    QString name = _info ? _info->_name : ui->user_name_lab->text();
    int maxW = ui->user_name_lab->contentsRect().width();
    if (maxW <= 0) maxW = ui->user_name_lab->maximumWidth();
    //if (maxW <= 0) maxW = 85;

    QFontMetrics fm(ui->user_name_lab->font());
    ui->user_name_lab->setText(fm.elidedText(name, Qt::ElideRight, maxW));
}
void ConUserItem::resizeEvent(QResizeEvent* e)
{
    // 当 item 宽度变了重新计算省略号
    updateMsgElide();
    updateNameElide();
    ListItemBase::resizeEvent(e);
}
ConUserItem::ConUserItem(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ConUserItem)
{
    ui->setupUi(this);
    setObjectName("chat_user_item");
    setAttribute(Qt::WA_StyledBackground, true);

    setAttribute(Qt::WA_StyledBackground, true);  // 允许 QSS 画背景
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
    SetItemType(ListItemType::CHAT_USER_ITEM);
    setStyleSheet(R"(
        QLabel#user_name_lab{
            font-size:14px;
            font-weight:600;
        color:#111111;
        }
        QLabel#user_chat_lab{
            font-size:12px;
            font-weight:400;
        color:#8A8A8A;
        }
        QLabel#time_lab{
            font-size:9px;
            font-weight:400;
        color:#B3B3B3;
        }
       )");

}

ConUserItem::~ConUserItem()
{
    delete ui;
}

void ConUserItem::SetInfo(int uid, QString name, QString icon)
{
    _info = std::make_shared<UserInfo>(uid, name, icon);

    //头像：裁圆 + 固定大小
    const int avatarSize = 36;
    ui->icon_lab->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(_info->_icon);
    ui->icon_lab->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->icon_lab->setScaledContents(false);

    ui->user_name_lab->setText(_info->_name);

    ui->user_state_lab->setWordWrap(false);//不换行
    QFontMetrics fm(ui->user_state_lab->font());
    const int maxTextWidth = 100;  //超出则变...
    QString show = fm.elidedText(_msg, Qt::ElideRight, maxTextWidth);
    ui->user_state_lab->setText(_msg);


    updateMsgElide();
    updateNameElide();
}

void ConUserItem::SetInfo(std::shared_ptr<AuthRsp> auth_rsp){
    _info = std::make_shared<UserInfo>(auth_rsp);

    //头像：裁圆 + 固定大小
    const int avatarSize = 36;
    ui->icon_lab->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(_info->_icon);
    ui->icon_lab->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->icon_lab->setScaledContents(false);

    ui->user_name_lab->setText(_info->_name);

    ui->user_state_lab->setWordWrap(false);//不换行
    QFontMetrics fm(ui->user_state_lab->font());
    const int maxTextWidth = 100;  //超出则变...
    QString show = fm.elidedText(_msg, Qt::ElideRight, maxTextWidth);
    ui->user_state_lab->setText(_msg);

    updateMsgElide();
    updateNameElide();
}

void ConUserItem::SetInfo(std::shared_ptr<AuthInfo> auth_info)
{
    _info = std::make_shared<UserInfo>(auth_info);
    //头像：裁圆 + 固定大小
    const int avatarSize = 36;
    ui->icon_lab->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(_info->_icon);
    ui->icon_lab->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->icon_lab->setScaledContents(false);

    ui->user_name_lab->setText(_info->_name);

    ui->user_state_lab->setWordWrap(false);//不换行
    QFontMetrics fm(ui->user_state_lab->font());
    const int maxTextWidth = 100;  //超出则变...
    QString show = fm.elidedText(_msg, Qt::ElideRight, maxTextWidth);
    ui->user_state_lab->setText(_msg);

    updateMsgElide();
    updateNameElide();
}

std::shared_ptr<UserInfo> ConUserItem::GetInfo()
{
    return _info;
}

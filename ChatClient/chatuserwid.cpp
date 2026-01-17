#include "chatuserwid.h"
#include "ui_chatuserwid.h"

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
void ChatUserWid::updateMsgElide()
{
    // 只显示一行，超出显示省略号
    ui->user_chat_lab->setWordWrap(false);

    // 当前 label 的“可用宽度”
    int maxW = ui->user_chat_lab->contentsRect().width();

    QFontMetrics fm(ui->user_chat_lab->font());
    ui->user_chat_lab->setText(fm.elidedText(_msg, Qt::ElideRight, maxW));
}

void ChatUserWid::updateNameElide()
{
    ui->user_name_lab->setWordWrap(false);

    int maxW = ui->user_name_lab->contentsRect().width();
    if (maxW <= 0) maxW = ui->user_name_lab->maximumWidth();
    //if (maxW <= 0) maxW = 85;

    QFontMetrics fm(ui->user_name_lab->font());
    ui->user_name_lab->setText(fm.elidedText(_name, Qt::ElideRight, maxW));
}
void ChatUserWid::resizeEvent(QResizeEvent* e)
{
    // 当 item 宽度变了重新计算省略号
    updateMsgElide();
    updateNameElide();
    ListItemBase::resizeEvent(e);
}
ChatUserWid::ChatUserWid(QWidget *parent) :
    ListItemBase(parent),
    ui(new Ui::ChatUserWid)
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

ChatUserWid::~ChatUserWid()
{
    delete ui;
}

void ChatUserWid::SetInfo(QString name, QString head, QString msg,QString time)
{
    _name = name;
    _head = head;
    _msg = msg;
    _time = time;
    // 加载图片
    QPixmap pixmap(_head);

    //头像：裁圆 + 固定大小
    const int avatarSize = 36;
    ui->icon_lab->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(head);
    ui->icon_lab->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->icon_lab->setScaledContents(false);

    ui->user_name_lab->setText(_name);

    ui->user_chat_lab->setWordWrap(false);//不换行
    QFontMetrics fm(ui->user_chat_lab->font());
    const int maxTextWidth = 100;  //超出则变...
    QString show = fm.elidedText(_msg, Qt::ElideRight, maxTextWidth);
    ui->user_chat_lab->setText(_msg);

    ui->time_lab->setText(_time);

    updateMsgElide();
}

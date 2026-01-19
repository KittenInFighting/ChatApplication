#include "applyfrienditem.h"
#include "ui_applyfrienditem.h"
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
void ApplyFriendItem::updateMsgElide()
{
    // 只显示一行，超出显示省略号
    ui->user_chat_lab->setWordWrap(false);

    // 当前 label 的“可用宽度”
    int maxW = ui->user_chat_lab->contentsRect().width();

    QFontMetrics fm(ui->user_chat_lab->font());
    ui->user_chat_lab->setText(fm.elidedText(_apply_info->_desc, Qt::ElideRight, maxW));
}

void ApplyFriendItem::updateNameElide()
{
    ui->user_name_lab->setWordWrap(false);

    int maxW = ui->user_name_lab->contentsRect().width();
    if (maxW <= 0) maxW = ui->user_name_lab->maximumWidth();
    //if (maxW <= 0) maxW = 85;

    QFontMetrics fm(ui->user_name_lab->font());
    ui->user_name_lab->setText(fm.elidedText(_apply_info->_name, Qt::ElideRight, maxW));
}
void ApplyFriendItem::resizeEvent(QResizeEvent* e)
{
    // 当 item 宽度变了重新计算省略号
    updateMsgElide();
    updateNameElide();
    ListItemBase::resizeEvent(e);
}
ApplyFriendItem::ApplyFriendItem(QWidget *parent) :
    ListItemBase(parent), _added(false),
    ui(new Ui::ApplyFriendItem)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_StyledBackground, true);
    setStyleSheet(R"(
    ApplyFriendItem QLabel#user_name_lab{
        color: rgb(0,153,255);
        font-size: 13px;
        font-weight: 400;
    }

    ApplyFriendItem QLabel#user_chat_lab{
        color: rgb(153,153,153);
        font-size: 12px;
        font-weight: 400;
    }
    ApplyFriendItem QLabel#already_add_lab{
        color: rgb(153,153,153);
        font-size: 12px;
        font-weight: 400;
    }
    ApplyFriendItem{
            background-color: #FFFFFF;
            border: 1px solid #E6E6E6;   /* 边框淡灰 */
            border-radius: 8px;          /* 圆角，按你喜好调 6~12 */
        }
    )");
    SetItemType(ListItemType::APPLY_FRIEND_ITEM);
    // ui->add_btn->SetState("normal","hover", "press");
    ui->add_btn->hide();
    connect(ui->add_btn, &QPushButton::clicked,  [this](){
        emit this->sig_auth_friend(_apply_info);
    });
}

ApplyFriendItem::~ApplyFriendItem()
{
    delete ui;
}

void ApplyFriendItem::SetInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;

    //头像：裁圆 + 固定大小
    const int avatarSize = 42;
    ui->user_icon_lab->setFixedSize(avatarSize, avatarSize);
    QPixmap pix(_apply_info->_icon);
    ui->user_icon_lab->setPixmap(makeRoundPixmap(pix, avatarSize));
    ui->user_icon_lab->setScaledContents(false);

    ui->user_name_lab->setText(_apply_info->_name);
    ui->user_chat_lab->setText(_apply_info->_desc);

}

void ApplyFriendItem::ShowAddBtn(bool bshow)
{
    if (bshow) {
        ui->add_btn->show();
        ui->already_add_lab->hide();
        _added = false;
    }
    else {
        ui->add_btn->hide();
        ui->already_add_lab->show();
        _added = true;
    }
}

int ApplyFriendItem::GetUid() {
    return _apply_info->_uid;
}

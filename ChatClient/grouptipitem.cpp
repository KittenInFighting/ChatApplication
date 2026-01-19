#include "grouptipitem.h"
#include "ui_grouptipitem.h"

GroupTipItem::GroupTipItem(QWidget *parent)
    : ListItemBase (parent),_tip("")
    , ui(new Ui::GroupTipItem)
{
    ui->setupUi(this);
    SetItemType(ListItemType::GROUP_TIP_ITEM);
}

GroupTipItem::~GroupTipItem()
{
    delete ui;
}

void GroupTipItem::SetGroupTip(QString str)
{
    ui->pushButton->setText(str);
}

QSize GroupTipItem::sizeHint() const
{
    return QSize(200, 35); // 返回自定义的尺寸
}

#include "adduseritem.h"
#include "ui_adduseritem.h"

AddUserItem::AddUserItem(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::AddUserItem)
{
    ui->setupUi(this);
}

AddUserItem::~AddUserItem()
{
    delete ui;
}

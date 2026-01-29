#include "whitepage.h"
#include "ui_whitepage.h"

WhitePage::WhitePage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::WhitePage)
{
    ui->setupUi(this);
}

WhitePage::~WhitePage()
{
    delete ui;
}

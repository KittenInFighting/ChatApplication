#include "lodingdlg.h"
#include "ui_lodingdlg.h"

LodingDlg::LodingDlg(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LodingDlg)
{
    ui->setupUi(this);
}

LodingDlg::~LodingDlg()
{
    delete ui;
}

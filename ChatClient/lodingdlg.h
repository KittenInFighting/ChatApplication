#ifndef LODINGDLG_H
#define LODINGDLG_H

#include <QDialog>

namespace Ui {
class LodingDlg;
}

class LodingDlg : public QDialog
{
    Q_OBJECT

public:
    explicit LodingDlg(QWidget *parent = nullptr);
    ~LodingDlg();

private:
    Ui::LodingDlg *ui;
};

#endif // LODINGDLG_H

#ifndef FINDSUCCESSDLG_H
#define FINDSUCCESSDLG_H

#include "userdata.h"
#include <memory>
#include <QDialog>

namespace Ui {
class FindSuccessDlg;
}

class FindSuccessDlg : public QDialog
{
    Q_OBJECT

public:
    explicit FindSuccessDlg(QWidget *parent = nullptr);
    ~FindSuccessDlg();
    void SetSearchInfo(std::shared_ptr<SearchInfo> si);
private slots:
    void on_add_friend_btn_clicked();

private:
    Ui::FindSuccessDlg *ui;
    std::shared_ptr<SearchInfo> _si;
    QWidget* _parent;
};

#endif // FINDSUCCESSDLG_H

#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include <QDialog>
#include "global.h"

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    explicit RegisterDialog(Ui::RegisterDialog *ui) : ui(ui) {}
    ~RegisterDialog();

private slots:
    void on_get_pushButton_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);

    void on_register_pushButton_clicked();

private:
    Ui::RegisterDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    void showTip(QString str,bool b_ok);
    void initHttpHandlers();
};

#endif // REGISTERDIALOG_H

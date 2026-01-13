#ifndef REGISTERDIALOG_H
#define REGISTERDIALOG_H

#include "global.h"
#include <QDialog>
#include <QLineEdit>
#include <QLabel>

namespace Ui {
class RegisterDialog;
}

class RegisterDialog : public QDialog
{
    Q_OBJECT

public:
    explicit RegisterDialog(QWidget *parent = nullptr);
    explicit RegisterDialog(Ui::RegisterDialog *ui) : ui(ui) {}
    void AddTipErr(TipErr te, QString tips);
    void DelTipErr(TipErr te);
    ~RegisterDialog();
protected:
    bool eventFilter(QObject* watched, QEvent* event) override;
private slots:
    void on_get_pushButton_clicked();
    void slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err);
    void on_register_pushButton_clicked();
private:
    QMap<TipErr, QString> _tip_errs;
    Ui::RegisterDialog *ui;
    QMap<ReqId, std::function<void(const QJsonObject&)>> _handlers;
    QIcon _eyeOpenIcon;
    QIcon _eyeCloseIcon;
    void showTip(QString str,bool b_ok);
    void initHttpHandlers();
    void checkNotEmpty(QLineEdit* edit, QLabel* errLabel, const QString& msg);
    bool clickedOutside(QLineEdit* edit, QObject* watchedObj) const;

    void checkPwd();                 // 检验密码
    QString pwdErrorMsg() const;     // 返回对应错误提示
    bool hasBadRepeatOrSequence(const QString& s) const; // 检验是否有连续重复6次以上的字母或数字
};

#endif // REGISTERDIALOG_H

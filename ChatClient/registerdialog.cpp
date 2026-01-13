#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog)
{
    ui->setupUi(this);
    ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);

    //设置qss里设置的erro_label属性
    ui -> erro_label -> setProperty("state","normal");
    repolish(ui -> erro_label);
    QObject::connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();
}

RegisterDialog::~RegisterDialog()
{
    delete ui;
}

void RegisterDialog::on_get_pushButton_clicked()
{
    auto email = ui->email_lineEdit->text();
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch();
    if(match){
        //发送验证码
        QJsonObject json_obj;
        json_obj["email"] = email;
        HttpMgr::GetInstance() -> PostHttpReq(QUrl(gate_url_prefix + "/get_verifycode"),
                                            json_obj, ReqId::ID_GET_VARIFY_CODE, Modules::REGISTERMOD);
        showTip(tr("验证码发送成功"),true);

    }else{
        showTip(tr("邮箱地址不正确"),false);
    }
}

void RegisterDialog::slot_reg_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"), false);
        return;
    }

    //解析JSON 字符串， res 转换为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    if(jsonDoc.isNull()){
        showTip(tr("json解析失败"),false);
        return;
    }
    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析失败"),false);
        return;
    }

    //
    auto it = _handlers.find(id);
    if (it == _handlers.end()) {
        showTip(tr("未知请求ID"), false);
        return;
    }
    it.value()(jsonDoc.object());
    return;
}

void RegisterDialog::showTip(QString str,bool b_ok)
{
    if(b_ok){
        ui->erro_label->setProperty("state","normal");
    }else{
        ui->erro_label->setProperty("state","err");

    }
    ui->erro_label->setText(str);
    repolish(ui->erro_label);
}

void RegisterDialog::initHttpHandlers()
{
    //注册获取验证码回包的逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](const QJsonObject& jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已经发送到邮箱"),true);
        qDebug() << "email is " << email;
    });

    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_REG_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("用户注册成功"), true);
        qDebug() << "user uuid is " << jsonObj["uid"].toString();
        qDebug() << "email is " << email ;
    });
}


void RegisterDialog::on_register_pushButton_clicked()
{
    if(ui->count_lineEdit->text() == ""){
        showTip(tr("用户名不能为空"), false);
        return;
    }

    if(ui->email_lineEdit->text() == ""){
        showTip(tr("邮箱不能为空"), false);
        return;
    }

    if(ui->pwd_lineEdit->text() == ""){
        showTip(tr("密码不能为空"), false);
        return;
    }

    if(ui->code_lineEdit->text() == ""){
        showTip(tr("验证码不能为空"), false);
        return;
    }

    //发送http请求注册用户
    QJsonObject json_obj;
    json_obj["user"] = ui->count_lineEdit->text();
    json_obj["email"] = ui->email_lineEdit->text();
    json_obj["passwd"] = ui->pwd_lineEdit->text();
    json_obj["verifycode"] = ui->code_lineEdit->text();
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_register"),
                                        json_obj, ReqId::ID_REG_USER,Modules::REGISTERMOD);
}


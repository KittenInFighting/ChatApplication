#include "resetdialog.h"
#include "ui_resetdialog.h"
#include <QDebug>
#include <QRegularExpression>
#include "global.h"
#include "httpmgr.h"

ResetDialog::ResetDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ResetDialog)
{
    ui->setupUi(this);

    connect(ui->user_edit,&QLineEdit::editingFinished,this,[this](){
        checkUserValid();
    });

    connect(ui->email_edit, &QLineEdit::editingFinished, this, [this](){
        checkEmailValid();
    });

    connect(ui->pwd_edit, &QLineEdit::editingFinished, this, [this](){
        checkPassValid();
    });


    connect(ui->verify_edit, &QLineEdit::editingFinished, this, [this](){
        checkVerifyValid();
    });

    //连接reset相关信号和注册处理回调
    initHandlers();
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reset_mod_finish, this,
            &ResetDialog::slot_reset_mod_finish);
}

ResetDialog::~ResetDialog()
{
    qDebug() << "ResetDialog destruct " << "\n";
    delete ui;
}

bool ResetDialog::checkUserValid()
{
    if(ui->user_edit->text() == ""){
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能为空"));
        return false;
    }

    //不能包含空格
    QRegularExpression reSpace("\\s");
    if (ui->user_edit->text().contains(reSpace)) {
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名不能包含空格"));
        return false;
    }
    QRegularExpression re("^[A-Za-z0-9_]+$");
    if (!re.match(ui->user_edit->text()).hasMatch()) {
        AddTipErr(TipErr::TIP_USER_ERR, tr("用户名只能包含字母、数字或下划线"));
        return false;
    }
    DelTipErr(TipErr::TIP_USER_ERR);
    return true;
}


bool ResetDialog::checkPassValid()
{
    auto pass = ui->pwd_edit->text();
    const QString textTrimmed = pass.trimmed();
    if(textTrimmed.isEmpty()){
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码不能为空"));
        return false;
    }

    if(pass.length() < 8 || pass.length()>16){
        //提示长度不准确
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码长度必须为8-16个字符"));
        return false;
    }
    QRegularExpression reSpace("\\s");
    if (pass.contains(reSpace)) {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码不能包含空格"));
        return false;
    }
    //不能包含连续/重复6位以上的字母或数字
    if (hasBadRepeatOrSequence(pass)) {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码不能包含连续或重复6位以上的字母/数字"));
        return false;
    }

    // 5) 必须包含【字母、数字、符号】中的至少两种
    bool hasLetter = false;
    bool hasDigit  = false;
    bool hasSymbol = false;

    for (const QChar ch : pass) {
        if (ch.isLetter()) {
            hasLetter = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        } else {
            // 这里把“非字母非数字”的都当作符号（因为已经排除了空白）
            hasSymbol = true;
        }
    }

    int categories = 0;
    if (hasLetter) categories++;
    if (hasDigit)  categories++;
    if (hasSymbol) categories++;

    if (categories < 2) {
        AddTipErr(TipErr::TIP_PWD_ERR, tr("密码必须包含字母/数字/符号中的至少两种"));
        return false;

    }

    DelTipErr(TipErr::TIP_PWD_ERR);

    return true;
}

bool ResetDialog::checkEmailValid()
{
    //验证邮箱的地址正则表达式
    auto email = ui->email_edit->text();
    // 邮箱地址的正则表达式
    QRegularExpression regex(R"((\w+)(\.|_)?(\w*)@(\w+)(\.(\w+))+)");
    bool match = regex.match(email).hasMatch(); // 执行正则表达式匹配
    if(!match){
        //提示邮箱不正确
        AddTipErr(TipErr::TIP_EMAIL_ERR, tr("邮箱地址不正确"));
        return false;
    }

    DelTipErr(TipErr::TIP_EMAIL_ERR);
    return true;
}

bool ResetDialog::checkVerifyValid()
{
    auto pass = ui->verify_edit->text();
    if(pass.isEmpty()){
        AddTipErr(TipErr::TIP_VERIFY_ERR, tr("验证码不能为空"));
        return false;
    }

    DelTipErr(TipErr::TIP_VERIFY_ERR);
    return true;
}

void ResetDialog::AddTipErr(TipErr te, QString tips)
{
    _tip_errs[te] = tips;
    showTip(tips, false);
}

void ResetDialog::DelTipErr(TipErr te)
{
    _tip_errs.remove(te);
    if(_tip_errs.empty()){
        ui->err_tip->clear();
        return;
    }

    showTip(_tip_errs.first(), false);
}

void ResetDialog::showTip(QString str, bool b_ok)
{
    if(b_ok){
        ui->err_tip->setProperty("state","normal");
    }else{
        ui->err_tip->setProperty("state","err");
    }

    ui->err_tip->setText(str);

    repolish(ui->err_tip);
}

void ResetDialog::on_verify_btn_clicked()
{
    qDebug()<<"receive verify btn clicked ";
    auto email = ui->email_edit->text();
    auto bcheck = checkEmailValid();
    if(!bcheck){
        ui -> verify_btn -> reset();
        return;
    }

    //发送http请求获取验证码
    QJsonObject json_obj;
    json_obj["email"] = email;
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/get_verifycode"),
                                        json_obj, ReqId::ID_GET_VARIFY_CODE,Modules::RESETMOD);
}

void ResetDialog::initHandlers()
{
    //注册获取验证码回包逻辑
    _handlers.insert(ReqId::ID_GET_VARIFY_CODE, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("验证码已发送到邮箱，注意查收"), true);
        qDebug()<< "email is " << email ;
    });

    //注册注册用户回包逻辑
    _handlers.insert(ReqId::ID_RESET_PWD, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
            showTip(tr("参数错误"),false);
            return;
        }
        auto email = jsonObj["email"].toString();
        showTip(tr("重置成功,点击返回登录"), true);
        qDebug()<< "email is " << email ;
        qDebug()<< "user uuid is " <<  jsonObj["uuid"].toString();
    });
}

bool ResetDialog::hasBadRepeatOrSequence(const QString& s)
{
    //检测重复：例如 111111 或 aaaaaa（连续相同字符 >= 6）
    int repeatCount = 1;
    for (int i = 1; i < s.size(); ++i) {
        const QChar prev = s[i - 1];
        const QChar curr = s[i];

        // 只针对字母/数字
        if (!prev.isLetterOrNumber() || !curr.isLetterOrNumber()) {
            repeatCount = 1;
            continue;
        }

        if (curr == prev) {
            repeatCount++;
            if (repeatCount >= 6) return true;
        } else {
            repeatCount = 1;
        }
    }

    // 检测递增连续：abcdefg、123456
    // - 字母按不区分大小写处理：a b c... / A B C... 都算连续
    // - 数字按 0-9 递增：1 2 3 4 5 6
    int seqCount = 1;

    auto normChar = [](QChar c) -> QChar {
        // 字母统一转小写，数字保持
        if (c.isLetter()) return c.toLower();
        return c;
    };

    for (int i = 1; i < s.size(); ++i) {
        QChar prev = s[i - 1];
        QChar curr = s[i];

        if (!prev.isLetterOrNumber() || !curr.isLetterOrNumber()) {
            seqCount = 1;
            continue;
        }

        prev = normChar(prev);
        curr = normChar(curr);

        // 必须同一类：都是数字 or 都是字母
        const bool bothDigit  = prev.isDigit()  && curr.isDigit();
        const bool bothLetter = prev.isLetter() && curr.isLetter();
        if (!bothDigit && !bothLetter) {
            seqCount = 1;
            continue;
        }

        // 判断是否递增连续：curr == prev + 1
        if (curr.unicode() == prev.unicode() + 1) {
            seqCount++;
            if (seqCount >= 6) return true;
        } else {
            seqCount = 1;
        }
    }

    return false;
}
void ResetDialog::slot_reset_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        showTip(tr("网络请求错误"),false);
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        showTip(tr("json解析错误"),false);
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        showTip(tr("json解析错误"),false);
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());

    return;
}
void ResetDialog::on_sure_btn_clicked()
{

        bool valid = checkUserValid();
        if(!valid){
            return;
        }

        valid = checkEmailValid();
        if(!valid){
            return;
        }

        valid = checkPassValid();
        if(!valid){
            return;
        }

        valid = checkVerifyValid();
        if(!valid){
            return;
        }

        //发送http重置用户请求
        QJsonObject json_obj;
        json_obj["user"] = ui->user_edit->text();
        json_obj["email"] = ui->email_edit->text();
        json_obj["passwd"] = QString(ui->pwd_edit->text());
        json_obj["verifycode"] = ui->verify_edit->text();
        HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/reset_pwd"),
                                            json_obj, ReqId::ID_RESET_PWD,Modules::RESETMOD);

}

void ResetDialog::on_return_btn_clicked()
{
    qDebug() << "return btn clicked ";
    emit switchLogin2();
}

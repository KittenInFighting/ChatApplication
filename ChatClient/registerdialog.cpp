#include "registerdialog.h"
#include "ui_registerdialog.h"
#include "global.h"
#include "httpmgr.h"
#include <QMouseEvent>
#include <QRegularExpression>

RegisterDialog::RegisterDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::RegisterDialog),_return(6)
{
    ui->setupUi(this);

    ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);
    //设置pwdEyeBtn
    QString openPath  = QDir::current().filePath("../../../res/eye_open.png");
    QString closePath = QDir::current().filePath("../../../res/eye_close.png");
    _eyeOpenIcon  = QIcon(openPath);
    _eyeCloseIcon = QIcon(closePath);
    ui->pwdEyeBtn->setIcon(_eyeCloseIcon);
    //初始设置不可见
    ui->pwdEyeBtn->setVisible(false);
    // 把按钮放进 lineEdit 里面
    ui->pwdEyeBtn->setParent(ui->pwd_lineEdit);
    ui->pwdEyeBtn->setStyleSheet("QToolButton{border:0;padding:0;}");
    // 固定位置到右侧中间
    const int btnSize = ui->pwdEyeBtn->sizeHint().height();
    ui->pwdEyeBtn->setFixedSize(btnSize, btnSize);
    const QRect r = ui->pwd_lineEdit->rect();
    const int x = r.right() - ui->pwdEyeBtn->width() - 4;
    const int y = (r.height() - ui->pwdEyeBtn->height()) / 2;
    ui->pwdEyeBtn->move(x, y);
    ui->pwdEyeBtn->raise();

    //设置pwd_tipWidget里面的提示内容
    ui->pwd_tip_noSpace->setText("不能包含空格");
    ui->pwd_tip_len->setText("长度为8-16个字符");
    ui->pwd_tip_kind->setText("必须包含字母、数字、符号中至少2种");
    ui->pwd_tip_seq->setText("请勿输入连续、重复6位以上字母或数字，如 abcdefg、111111、0123456");

    // 初始errLabel
    ui->count_errLabel->hide();
    ui->email_errLabel->hide();
    ui->pwd_errLabel->hide();
    ui->count_errLabel->setStyleSheet("color:#ff4d4f; font-size:11px;");
    ui->email_errLabel->setStyleSheet("color:#ff4d4f; font-size:11px;");
    ui->pwd_errLabel->setStyleSheet("color:#ff4d4f; font-size:11px;");

    //初始pwd提示widget
    ui->pwd_tipWidget->hide();

    //初始化注册成功返回的定时器
    _return_timer = new QTimer(this);

    //初始化return_label
    ui->return_label->setStyleSheet("font-size:18px;");
    ui->return_label->setTextFormat(Qt::RichText);

    //监听lineEdit 的焦点离开事件：处理切到其他lineEdit
    ui->count_lineEdit->installEventFilter(this);
    ui->email_lineEdit->installEventFilter(this);
    ui->pwd_lineEdit->installEventFilter(this);
    // 安装全局事件过滤器：监听全局鼠标点击空白处
    qApp->installEventFilter(this);

    //设置qss里设置的erro_label属性
    ui -> erro_label -> setProperty("state","normal");
    repolish(ui -> erro_label);
    QObject::connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_reg_mod_finish, this, &RegisterDialog::slot_reg_mod_finish);
    initHttpHandlers();

    // 输入过程中如果不出错就立刻收起错误提示
    connect(ui->count_lineEdit, &QLineEdit::textChanged, this, [this](const QString&) {
        if (!ui->count_lineEdit->text().trimmed().isEmpty()) {
            ui->count_errLabel->hide();
            ui->count_lineEdit->setStyleSheet("");
        }
    });
    connect(ui->email_lineEdit, &QLineEdit::textChanged, this, [this](const QString&) {
        if (!ui->email_lineEdit->text().trimmed().isEmpty()) {
            ui->email_errLabel->hide();
            ui->email_lineEdit->setStyleSheet("");
        }
    });
    connect(ui->pwd_lineEdit, &QLineEdit::textChanged, this, [this](const QString&) {
        if (pwdErrorMsg().isEmpty()) {
            ui->pwd_errLabel->hide();
            ui->pwd_lineEdit->setStyleSheet("");
        }
        if (!ui->pwd_tipWidget->isVisible())ui->pwd_tipWidget->show();
    });

    //设置pwdEyeBtn的槽函数
    connect(ui->pwdEyeBtn, &QToolButton::pressed, this, [this]() {
        ui->pwdEyeBtn->setIcon(_eyeOpenIcon);
        ui->pwd_lineEdit->setEchoMode(QLineEdit::Normal);

        // 保持光标在末尾
        // ui->pwd_lineEdit->setFocus();
        // ui->pwd_lineEdit->setCursorPosition(ui->pwd_lineEdit->text().length());
    });
    connect(ui->pwdEyeBtn, &QToolButton::released, this, [this]() {
        ui->pwdEyeBtn->setIcon(_eyeCloseIcon);
        ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);

        ui->pwd_lineEdit->setFocus();
        ui->pwd_lineEdit->setCursorPosition(ui->pwd_lineEdit->text().length());
    });

    connect(ui->pwd_lineEdit, &QLineEdit::textChanged, this, [this](const QString& text) {
        const bool empty = text.isEmpty();

        ui->pwdEyeBtn->setVisible(!empty);

        // 如果清空了密码，回到“隐藏模式”，避免还处于明文状态
        if (empty) {
            ui->pwdEyeBtn->setIcon(_eyeCloseIcon); // closeIcon 你初始化时保存成成员
            ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);    
        }
    });

    //连接注册返回定时器信号槽
    connect(_return_timer, &QTimer::timeout, [this](){
        if(_return==0){
            _return_timer->stop();
            emit sigSwitchLogin();
            return;
        }
        _return--;
        //auto str = QString("%1 s后注册界面将自动关闭").arg(_return);
        ui->return_label->setText(
            QString("<span style='color:green;'>%1</span>"
                    "<span style='color:red;'> s后注册界面将自动关闭</span>")
                .arg(_return)
            );

    });

}

RegisterDialog::~RegisterDialog()
{
    if (ui->stackedWidget->currentWidget() != ui->page_1) {
        ui->stackedWidget->setCurrentWidget(ui->page_1);//切回page_1
    }
    qDebug() << "destruct RegisterDlg" << "\n";
    qApp->removeEventFilter(this);
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
        ui -> get_pushButton -> reset();
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
        ChangePage();
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

bool RegisterDialog::eventFilter(QObject* watched, QEvent* event)
{
    // 密码提示tip pwd_lineEdit：获得焦点就显示提示
    if (watched == ui->pwd_lineEdit && event->type() == QEvent::FocusIn) {
        ui->pwd_tipWidget->show();
        ui->pwd_errLabel->hide();
        return QDialog::eventFilter(watched, event);
    }
    // 1. FocusOut：从当前输入框点击到其它输入框，一定触发
    if (event->type() == QEvent::FocusOut) {
        if (watched == ui->count_lineEdit) {
            checkNotEmpty(ui->count_lineEdit, ui->count_errLabel, "账号不可以为空");
        } else if (watched == ui->email_lineEdit) {
            checkNotEmpty(ui->email_lineEdit, ui->email_errLabel, "请输入邮箱地址");
        }else if (watched == ui->pwd_lineEdit) {
            ui->pwd_tipWidget->hide();
            checkPwd();
        }
        return QDialog::eventFilter(watched, event);
    }

    // 2. MousePress：点击空白/不可聚焦控件时补齐（FocusOut 可能不发生）
    if (event->type() == QEvent::MouseButtonPress) {

        // 校验 count
        if (ui->count_lineEdit->hasFocus() && clickedOutside(ui->count_lineEdit, watched)) {
            checkNotEmpty(ui->count_lineEdit, ui->count_errLabel, "账号不可以为空");
        }

        // 校验 email
        if (ui->email_lineEdit->hasFocus() && clickedOutside(ui->email_lineEdit, watched)) {
            checkNotEmpty(ui->email_lineEdit, ui->email_errLabel, "邮箱不可以为空");
        }

        //校验密码
        if (ui->pwd_lineEdit->hasFocus()) {
            QWidget* w = qobject_cast<QWidget*>(watched);
            if (w && w != ui->pwd_lineEdit && !ui->pwd_lineEdit->isAncestorOf(w)) {
                ui->pwd_tipWidget->hide();
                checkPwd();
            }
        }
    }

    return QDialog::eventFilter(watched, event);
}

void RegisterDialog::checkNotEmpty(QLineEdit* edit, QLabel* errLabel, const QString& msg)
{
    // trimmed() 防止只输入空格也被当作“有内容”
    const QString text = edit->text().trimmed();

    if (text.isEmpty()) {
        // 显示错误 => 布局挤开
        errLabel->setText(msg);
        errLabel->show();

        // 输入框红边框（可选）
        edit->setStyleSheet("border:1px solid #ff4d4f;");
    } else {
        // 隐藏错误 => 布局回归
        errLabel->hide();
        edit->setStyleSheet("");
    }
}

bool RegisterDialog::clickedOutside(QLineEdit* edit, QObject* watchedObj) const
{
    // watchedObj 是被点击的控件（qApp 事件过滤器里会传各种对象）
    QWidget* w = qobject_cast<QWidget*>(watchedObj);
    if (!w) return false;

    // 如果点击的是 edit 本身或其子控件（一般 edit 没子控件，但写全更稳），就认为没“点出去”
    if (w == edit || edit->isAncestorOf(w)) return false;

    return true;
}

void RegisterDialog::checkPwd()
{
    const QString msg = pwdErrorMsg();

    if (!msg.isEmpty()) {
        // 显示错误：会挤下面布局
        ui->pwd_errLabel->setText(msg);
        ui->pwd_errLabel->show();

        // 输入框红边框
        ui->pwd_lineEdit->setStyleSheet("border:1px solid #ff4d4f;");
    } else {
        // 通过：隐藏错误（布局回归）
        ui->pwd_errLabel->hide();
        ui->pwd_lineEdit->setStyleSheet("");
    }
}

// 返回第一条命中的错误提示；返回 "" 表示通过
QString RegisterDialog::pwdErrorMsg() const
{
    // 取文本并去掉前后空格（注意：规则3禁止空格，这里 trimmed 只为判断空内容）
    const QString raw = ui->pwd_lineEdit->text();
    const QString textTrimmed = raw.trimmed();

    // 1) 不能为空
    if (textTrimmed.isEmpty()) {
        return "密码不能为空";
    }

    // 2) 长度 8-16
    //    注意：这里按“字符数”统计。若你要严格按字节，可改用 toUtf8().size()
    const int len = raw.length();
    if (len < 8 || len > 16) {
        return "密码长度必须为8-16个字符";
    }

    // 3) 不能包含空格（包含任何空白字符：空格/tab/换行等）
    static QRegularExpression reSpace("\\s");
    if (raw.contains(reSpace)) {
        return "密码不能包含空格";
    }

    // 4) 不能包含连续/重复6位以上的字母或数字
    if (hasBadRepeatOrSequence(raw)) {
        return "密码不能包含连续或重复6位以上的字母/数字";
    }

    // 5) 必须包含【字母、数字、符号】中的至少两种
    bool hasLetter = false;
    bool hasDigit  = false;
    bool hasSymbol = false;

    for (const QChar ch : raw) {
        if (ch.isLetter()) {
            hasLetter = true;
        } else if (ch.isDigit()) {
            hasDigit = true;
        } else {
            // 这里把“非字母非数字”的都当作符号（因为规则3已经排除了空白）
            hasSymbol = true;
        }
    }

    int categories = 0;
    if (hasLetter) categories++;
    if (hasDigit)  categories++;
    if (hasSymbol) categories++;

    if (categories < 2) {
        return "密码必须包含字母/数字/符号中的至少两种";
    }

    // 全部通过
    return "";
}

bool RegisterDialog::hasBadRepeatOrSequence(const QString& s) const
{
    // --- A) 检测重复：例如 111111 或 aaaaaa（连续相同字符 >= 6）
    int repeatCount = 1;
    for (int i = 1; i < s.size(); ++i) {
        const QChar prev = s[i - 1];
        const QChar curr = s[i];

        // 只针对字母/数字（按你的规则）
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

    // 检测递增连续：abcdefg、123456（连续递增 >= 6）
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

void RegisterDialog::ChangePage()
{
    _return = 6;
    ui->return_label->setTextFormat(Qt::RichText);
    ui->return_label->setText(
                        QString("<span style='color:green;'>%1</span>"
                                "<span style='color:red;'> s后注册界面将自动关闭</span>")
            .arg(_return));

    ui->return_label->show();
    ui->return_label->update(); // 立即刷新

    _return_timer->stop();
    ui->stackedWidget->setCurrentWidget(ui->page_2);

    // 启动定时器，设置间隔为1000毫秒（1秒）
    _return_timer->start(1000);
}



void RegisterDialog::on_return_pushBtn_clicked()
{
    _return_timer->stop();
    emit sigSwitchLogin();
}

void RegisterDialog::setupPwdEyeButton()
{
    QLineEdit* edit = ui->pwd_lineEdit;
    if (!edit) return;

    // 1) 代码创建一个 QToolButton，名字必须和你要求的一样
    auto* btn = new QToolButton(edit);                // parent 设为 lineEdit => “嵌入”到输入框内部
    btn->setObjectName("pwdEyeBtn");                  // 名字相同
    btn->setCursor(Qt::PointingHandCursor);

    // 2) 外观：去边框（像内嵌图标按钮）
    btn->setStyleSheet("QToolButton{border:none;padding:0px;}");

    // 3) 大小：按你图标/界面调整
    btn->setFixedSize(24, 24);

    // 4) 给输入框右侧留空，避免文字盖住按钮
    const int rightMargin = btn->width() + 8;
    edit->setTextMargins(0, 0, rightMargin, 0);

    // 5) 放到输入框内部右侧并垂直居中（你说长度固定，这样一次定位就够）
    const int paddingRight = 4;
    int x = edit->rect().right() - btn->width() - paddingRight;
    int y = (edit->height() - btn->height()) / 2;
    btn->move(x, y);
    btn->raise();
}


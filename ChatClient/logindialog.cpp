#include "logindialog.h"
#include "ui_logindialog.h"
#include "httpmgr.h"
#include "global.h"
#include "tcpmgr.h"
#include <QPainter>
#include <QPainterPath>
#include <QResizeEvent>
#include <QCloseEvent>
#include <QDebug>
#include <QRegularExpression>
#include <QMessageBox>
#include <QTimer>
#include <QPropertyAnimation>
#include <QEasingCurve>
#include <QAbstractAnimation>

LoginDialog::LoginDialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    initHttpHandlers();
    //设置窗口
    setWindowFlags(Qt::FramelessWindowHint | Qt::Dialog);
    setAttribute(Qt::WA_TranslucentBackground, true);
    setAttribute(Qt::WA_StyledBackground, true);

    //设置窗口右上方的closeBtn
    connect(ui->btnClose, &QToolButton::clicked, this, &LoginDialog::close);

    //初始禁用登录按钮
    ui->signIn_pushButton->setEnabled(false);
    // 让头像 QLabel 固定成正方形
    ui->head_label->setFixedSize(80, 80);
    ui->head_label->setScaledContents(false);

    // 2) 设置一张头像
    setAvatar(":/res/1.jpg");

    // 让 label 不吃背景
    ui->head_label->setStyleSheet("background: transparent;");

    //geiregister_label绑定点击事件
    ui -> register_label -> setCursor(Qt::PointingHandCursor);
    connect(ui->register_label,&ClickableLabel::clicked,this,&LoginDialog::switchRegister);

    //forget_label绑定点击事件
    ui->forget_label->setCursor(Qt::PointingHandCursor);
    connect(ui->forget_label, &ClickableLabel::clicked, this, &LoginDialog::slot_forget_pwd);

    //设置LineEdit文字居中
    ui->count_lineEdit->setAlignment(Qt::AlignHCenter);
    ui->pwd_lineEdit->setAlignment(Qt::AlignHCenter);

    //设置密码不可见
    ui->pwd_lineEdit->setEchoMode(QLineEdit::Password);

    //每次lineEdit变动判断是否满足双非空的sign_pushButton的激活条件
    auto updateSignInState = [this](const QString &) {
        bool ready = !ui->count_lineEdit->text().trimmed().isEmpty()
                     &&!ui->pwd_lineEdit->text().trimmed().isEmpty();
        ui->signIn_pushButton->setEnabled(ready);
    };
    connect(ui->count_lineEdit, &QLineEdit::textChanged, this, updateSignInState);
    connect(ui->pwd_lineEdit, &QLineEdit::textChanged, this, updateSignInState);

    //连接登录回包信号，收到信号开始处理回包
    connect(HttpMgr::GetInstance().get(), &HttpMgr::sig_login_mod_finish, this,
            &LoginDialog::slot_login_mod_finish);

    //连接tcp连接请求的信号和槽函数
    connect(this, &LoginDialog::sig_connect_tcp, TcpMgr::GetInstance().get(), &TcpMgr::slot_tcp_connect);
    //连接tcp管理者发出的连接成功信号
    connect(TcpMgr::GetInstance().get(), &TcpMgr::sig_con_success, this, &LoginDialog::slot_tcp_con_finish);
}

LoginDialog::~LoginDialog()
{
    qDebug() << "destruct LoginDlg" << "\n";
    delete ui;
}

void LoginDialog::setAvatar(const QString &path)
{
    _avatarSrc = QPixmap(path);
    updateAvatar();
}

void LoginDialog::resizeEvent(QResizeEvent *event)
{
    QDialog::resizeEvent(event);

    // 如果你的 label头像 不是 fixedSize，而是会跟布局变化，这里能保证始终是圆的
    updateAvatar();
}

void LoginDialog::closeEvent(QCloseEvent *event)
{
    emit sigLoginClosed();
    QDialog::closeEvent(event);
}

void LoginDialog::updateAvatar()
{
    if (_avatarSrc.isNull() || !ui->head_label) return;

    int d = qMin(ui->head_label->width(), ui->head_label->height());
    if (d <= 0) return;

    QPixmap round = makeRoundPixmap(_avatarSrc, d,3);
    ui->head_label->setPixmap(round);
}

QPixmap LoginDialog::makeRoundPixmap(const QPixmap &src, int diameter,int borderWidth)
{
    if (src.isNull() || diameter <= 0) return QPixmap();

    // 先缩放到足够大，再裁剪，保证清晰
    QPixmap scaled = src.scaled(diameter, diameter,
                                Qt::KeepAspectRatioByExpanding,
                                Qt::SmoothTransformation);

    QPixmap dst(diameter, diameter);
    dst.fill(Qt::transparent);

    QPainter p(&dst);
    p.setRenderHint(QPainter::Antialiasing, true);
    p.setRenderHint(QPainter::SmoothPixmapTransform, true);

    // 圆形裁剪
    QPainterPath path;
    path.addEllipse(0, 0, diameter, diameter);
    p.setClipPath(path);

    // 居中绘制
    int x = (diameter - scaled.width()) / 2;
    int y = (diameter - scaled.height()) / 2;
    p.drawPixmap(x, y, scaled);

    // 画白色描边
    p.setClipping(false);
    QPen pen(QColor(255, 255, 255, 255), borderWidth);
    p.setPen(pen);
    p.setBrush(Qt::NoBrush);

    // 让线条完整显示在图内
    const int half = borderWidth / 2;
    p.drawEllipse(half, half, diameter - borderWidth, diameter - borderWidth);
    return dst;
}

bool LoginDialog::checkCount()
{
    const QString raw = ui->count_lineEdit->text();
    const QString trimmed = raw.trimmed();

    if (trimmed.isEmpty()) {
        return false;
    }

    if (raw.contains(QRegularExpression("\\s"))) {
        return false;
    }

    QRegularExpression re("^[A-Za-z0-9_]+$");
    if (!re.match(raw).hasMatch()) {
        return false;
    }

    ui->count_lineEdit->setStyleSheet("");
    return true;
}

bool LoginDialog::checkPwd()
{
    // 取文本并去掉前后空格
    const QString raw = ui->pwd_lineEdit->text();
    const QString textTrimmed = raw.trimmed();

    // 1) 不能为空
    if (textTrimmed.isEmpty()) {
        return false;
    }

    // 2) 长度 8-16
    const int len = raw.length();
    if (len < 8 || len > 16) {
        return false;
    }

    // 3) 不能包含空格（包含任何空白字符：空格/tab/换行等）
    static QRegularExpression reSpace("\\s");
    if (raw.contains(reSpace)) {
        return false;
    }

    // 4) 不能包含连续/重复6位以上的字母或数字
    if (hasBadRepeatOrSequence(raw)) {
        return false;
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
            // 这里把“非字母非数字”的都当作符号
            hasSymbol = true;
        }
    }

    int categories = 0;
    if (hasLetter) categories++;
    if (hasDigit)  categories++;
    if (hasSymbol) categories++;

    if (categories < 2) {
        return false;
    }

    // 全部通过
    return true;
}

void LoginDialog::initHttpHandlers()
{
    //注册获取登录回包逻辑
    _handlers.insert(ReqId::ID_LOGIN_USER, [this](QJsonObject jsonObj){
        int error = jsonObj["error"].toInt();
        if(error != ErrorCodes::SUCCESS){
           // 添加错误处理,可考虑对齐服务端错误代码，提示具体错误，但不能提示具体账号或密码错误
            QMessageBox::warning(this, tr("登录失败"), tr("参数错误"));
            return;
        }
        auto user = jsonObj["user"].toString();
        //登录账号密码验证成功，进一步处理
        //QMessageBox::information(this, tr("提示"), tr("登录成功"));
        //qDebug()<< "user is " << user ;

        //发送信号通知tcpMgr发送长链接
        ServerInfo si;
        si.Uid = jsonObj["uid"].toInt();
        si.Host = jsonObj["host"].toString();
        si.Port = jsonObj["port"].toString();
        si.Token = jsonObj["token"].toString();
        if(si.Host == nullptr || si.Port==nullptr)
        {
            //收到的服务器端口和ip为空必然失败，失败提示
            qDebug()<<"Host or IP is null" << "\n";
            QMessageBox::warning(this, tr("tcp连接失败"), tr("请检查网络"));
            return;
        }
        _uid = si.Uid;
        _token = si.Token;
        qDebug()<< "user is " << user << " uid is " << si.Uid <<" host is "
                 << si.Host << " Port is " << si.Port << " Token is " << si.Token;
        emit sig_connect_tcp(si);
    });

}

void LoginDialog::showLoginErrorDialog()
{

    // 黑色遮罩覆盖LoginDialog背景
    auto* mask = new RoundedMask(this, 16);
    mask->setGeometry(rect());
    mask->show();
    mask->raise();

    // 2) 圆角弹窗,透明背景 + 圆角内容面板
    QDialog dlg(this);
    dlg.resize(280, 150);
    dlg.setWindowFlags(Qt::Dialog | Qt::FramelessWindowHint);
    dlg.setAttribute(Qt::WA_TranslucentBackground, true);
    dlg.setModal(true);

    // 计算最终位置
    const QPoint center = this->mapToGlobal(this->rect().center());

    const QPoint endPos = center - QPoint(dlg.width() / 2, dlg.height() / 2);
    // 起始位置（上移 12 像素）
    const QPoint startPos = endPos + QPoint(0, -12);

    // 先放到起始位置
    dlg.move(startPos);

    // 进场动画：从上往下
    QTimer::singleShot(0, &dlg, [startPos, endPos, &dlg]() {
        auto* anim = new QPropertyAnimation(&dlg, "pos", &dlg);
        anim->setDuration(220);
        anim->setStartValue(startPos);
        anim->setEndValue(endPos);
        anim->setEasingCurve(QEasingCurve::OutCubic);
        anim->start(QAbstractAnimation::DeleteWhenStopped);
    });

    auto* panel = new QWidget(&dlg);
    panel->setObjectName("panel");
    panel->setStyleSheet("#panel{background:#ffffff;border-radius:12px;}");

    //绘制窗口边框阴影
    auto* shadow = new QGraphicsDropShadowEffect(panel);
    shadow->setBlurRadius(20);
    shadow->setColor(QColor(0,0,0,80));
    shadow->setOffset(0,4);
    panel->setGraphicsEffect(shadow);

    auto* outer = new QVBoxLayout(&dlg);
    outer->setContentsMargins(12, 12, 12, 12); // 给阴影留空间
    outer->addWidget(panel);

    auto* layout = new QVBoxLayout(panel);
    layout->setContentsMargins(16, 8, 8, 16);
    layout->setSpacing(12);

    // 顶部行：右上角关闭按钮
    auto* topRow = new QHBoxLayout;
    auto* closeBtn = new QToolButton(panel);
    closeBtn->setText("×");
    closeBtn->setAutoRaise(true);
    closeBtn->setCursor(Qt::PointingHandCursor);
    closeBtn->setFixedSize(24, 24);
    closeBtn->setStyleSheet(
        "QToolButton{border:none;font-size:16px;color:#666;}"
        "QToolButton:hover{color:#000;}"
        );
    connect(closeBtn, &QToolButton::clicked, &dlg, &QDialog::reject);
    topRow->addStretch();
    topRow->addWidget(closeBtn);
    layout->addLayout(topRow);

    auto* label = new QLabel(tr("账号或密码错误，请重新输入。"), panel);
    label->setAlignment(Qt::AlignCenter);

    auto* btn = new QPushButton(tr("重新登录"), panel);
    connect(btn, &QPushButton::clicked, &dlg, &QDialog::accept);

    layout->addWidget(label);
    layout->addWidget(btn, 0, Qt::AlignHCenter);


    dlg.move(startPos);
    dlg.exec();

    // 弹窗关闭后隐藏遮罩
    mask->deleteLater();
}

bool LoginDialog::hasBadRepeatOrSequence(const QString& s) const
{
    //检测重复：例如 111111 或 aaaaaa（连续相同字符 >= 6）
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

void LoginDialog::mousePressEvent(QMouseEvent *e)
{
    if (e->button() == Qt::LeftButton) {
        QWidget *child = childAt(e->pos());
        if (child && (child == ui->signIn_pushButton || ui->signIn_pushButton->isAncestorOf(child))) {
            m_dragging = false;
            return;
        }
        m_dragging = true;
        m_dragPos = e->globalPosition().toPoint() - frameGeometry().topLeft();
        e->accept();
    }
}

void LoginDialog::mouseReleaseEvent(QMouseEvent *e)
{
    m_dragging = false;
    QDialog::mouseReleaseEvent(e);
}

void LoginDialog::slot_forget_pwd()
{
    qDebug()<<"slot forget pwd";
    emit switchReset();
}

void LoginDialog::slot_login_mod_finish(ReqId id, QString res, ErrorCodes err)
{
    if(err != ErrorCodes::SUCCESS){
        //实现(tr("网络请求错误"),false);
        QMessageBox::warning(this, tr("登录失败"), tr("网络请求错误"));
        return;
    }

    // 解析 JSON 字符串,res需转化为QByteArray
    QJsonDocument jsonDoc = QJsonDocument::fromJson(res.toUtf8());
    //json解析错误
    if(jsonDoc.isNull()){
        //实现(tr("json解析错误"),false);
        QMessageBox::warning(this, tr("登录失败"), tr("json解析错误"));
        return;
    }

    //json解析错误
    if(!jsonDoc.isObject()){
        //实现(tr("json解析错误"),false);
        QMessageBox::warning(this, tr("登录失败"), tr("json解析错误"));
        return;
    }


    //调用对应的逻辑,根据id回调。
    _handlers[id](jsonDoc.object());
    return;
}

void LoginDialog::slot_tcp_con_finish(bool bsuccess)
{

    if(bsuccess){
        QMessageBox::information(this, tr("提示"), tr("登录成功"));
        QJsonObject jsonObj;
        jsonObj["uid"] = _uid;
        jsonObj["token"] = _token;

        QJsonDocument doc(jsonObj);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        //发送tcp请求给chat server
        TcpMgr::GetInstance()->sig_send_data(ReqId::ID_CHAT_LOGIN, jsonString);

    }else{
            QMessageBox::warning(this, tr("连接失败"), tr("请检查网络"));
    }

}

void LoginDialog::mouseMoveEvent(QMouseEvent *e)
{
    if (e->buttons() & Qt::LeftButton && m_dragging) {
        move(e->globalPosition().toPoint() - m_dragPos);
        e->accept();
    }
}

void LoginDialog::on_signIn_pushButton_clicked()
{
    qDebug()<<"login btn clicked";
    if(checkCount() == false || checkPwd() == false){
        qDebug()<<"Count or Pwd Error" << "\n";
        showLoginErrorDialog();
        return;
    }

    // if(checkPwd() == false){
    //     qDebug()<<"Pwd Error" << "\n";
    //     return ;
    // }

    auto user = ui->count_lineEdit->text();
    auto pwd = ui->pwd_lineEdit->text();
    //发送http短连接向GateServer请求登录
    QJsonObject json_obj;
    json_obj["user"] = user;
    json_obj["passwd"] = QString(pwd);
    HttpMgr::GetInstance()->PostHttpReq(QUrl(gate_url_prefix+"/user_login"),
                                        json_obj, ReqId::ID_LOGIN_USER,Modules::LOGINMOD);
}


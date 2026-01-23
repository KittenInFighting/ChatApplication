#include "authenfriend.h"
#include "ui_authenfriend.h"
#include "usermgr.h"
#include "tcpmgr.h"
#include <QScrollBar>

AuthenFriend::AuthenFriend(QWidget *parent)
    : QDialog()
    , ui(new Ui::AuthenFriend)
{
    ui->setupUi(this);

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);

    connect(ui->cancel_btn, &QPushButton::clicked, this, &AuthenFriend::SlotAuthenCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &AuthenFriend::SlotAuthenSure);
}

AuthenFriend::~AuthenFriend()
{
    delete ui;
}

bool AuthenFriend::eventFilter(QObject *obj, QEvent *event)
{

    if (obj == ui->scrollArea && event->type() == QEvent::Enter)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(false);
    }
    else if (obj == ui->scrollArea && event->type() == QEvent::Leave)
    {
        ui->scrollArea->verticalScrollBar()->setHidden(true);
    }
    return QObject::eventFilter(obj, event);

}

void AuthenFriend::SetApplyInfo(std::shared_ptr<ApplyInfo> apply_info)
{
    _apply_info = apply_info;
    ui->memoname_edit->setPlaceholderText(apply_info->_name);
}

void AuthenFriend::SlotAuthenCancel()
{
    qDebug() << "Slot Authen Cancel";
    this->hide();
    deleteLater();
}

void AuthenFriend::SlotAuthenSure()
{
    qDebug() << "Slot Apply Sure ";
    //添加发送逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::GetInstance()->GetUid();
    jsonObj["fromuid"] = uid;
    jsonObj["touid"] = _apply_info->_uid;
    QString back_name = "";
    if(ui->memoname_edit->text().isEmpty()){
        back_name = ui->memoname_edit->placeholderText();
    }else{
        back_name = ui->memoname_edit->text();
    }
    jsonObj["back"] = back_name;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_AUTH_FRIEND_REQ, jsonData);

    this->hide();
    deleteLater();
}


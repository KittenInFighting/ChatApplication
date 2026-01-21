#include "applyfriend.h"
#include "ui_applyfriend.h"
#include "usermgr.h"
#include "tcpmgr.h"
#include <QScrollBar>
ApplyFriend::ApplyFriend(QWidget *parent)
    : QDialog()
    , ui(new Ui::ApplyFriend)
{
    ui->setupUi(this);

    ui->scrollArea->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    ui->scrollArea->horizontalScrollBar()->setHidden(true);
    ui->scrollArea->verticalScrollBar()->setHidden(true);
    ui->scrollArea->installEventFilter(this);

    connect(ui->cancel_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplyCancel);
    connect(ui->sure_btn, &QPushButton::clicked, this, &ApplyFriend::SlotApplySure);
}

ApplyFriend::~ApplyFriend()
{
    delete ui;
}

bool ApplyFriend::eventFilter(QObject *obj, QEvent *event)
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

void ApplyFriend::SetSearchInfo(std::shared_ptr<SearchInfo> si)
{
    _si = si;
    auto applyname = UserMgr::GetInstance()->GetName();
    auto memoname = si->_name;
    ui->name_edit->setText(applyname);
    ui->memoname_edit->setText(memoname);
}

void ApplyFriend::SlotApplyCancel()
{
    qDebug() << "Slot Apply Cancel";
    this->hide();
    deleteLater();
}

void ApplyFriend::SlotApplySure()
{
    qDebug()<<"Slot Apply Sure called" ;
    //发送请求逻辑
    QJsonObject jsonObj;
    auto uid = UserMgr::GetInstance()->GetUid();
    jsonObj["uid"] = uid;
    auto name = ui->name_edit->text();
    if(name.isEmpty()){
        name = ui->name_edit->placeholderText();
    }

    jsonObj["applyname"] = name;

    auto bakname = ui->memoname_edit->text();
    if(bakname.isEmpty()){
        bakname = ui->memoname_edit->placeholderText();
    }

    jsonObj["bakname"] = bakname;
    jsonObj["touid"] = _si->_uid;

    QJsonDocument doc(jsonObj);
    QByteArray jsonData = doc.toJson(QJsonDocument::Compact);

    //发送tcp请求给chat server
    emit TcpMgr::GetInstance()->sig_send_data(ReqId::ID_ADD_FRIEND_REQ, jsonData);
    this->hide();
    deleteLater();
}

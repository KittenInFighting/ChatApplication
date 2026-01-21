#ifndef TCPMGR_H
#define TCPMGR_H

#include "userdata.h"
#include "singleton.h"
#include "global.h"
#include <QTcpSocket>


class TcpMgr:public QObject, public Singleton<TcpMgr>,
               public std::enable_shared_from_this<TcpMgr>
{
    Q_OBJECT
public:
    TcpMgr();
    ~ TcpMgr();
private:
    void initHandlers();
    void handleMsg(ReqId id, int len, QByteArray data);
    QTcpSocket _socket;
    QString _host;
    uint16_t _port;
    QByteArray _buffer;
    bool _b_recv_pending;
    quint16 _message_id;
    quint16 _message_len;
    QMap<ReqId, std::function<void(ReqId id, int len, QByteArray data)>> _handlers;
public slots:
    void slot_tcp_connect(ServerInfo);//处理连接
    void slot_send_data(ReqId reqId, QByteArray dataBytes);//发送数据
signals:
    void sig_con_success(bool bsuccess);//连接成功信号
    void sig_send_data(ReqId reqId, QByteArray data);//发送数据信号
    void sig_login_failed(int);//登录失败信号
    void sig_user_search(std::shared_ptr<SearchInfo>);//搜索
    void sig_friend_apply(std::shared_ptr<AddFriendApply>);//收到好友申请
    void sig_add_auth_friend(std::shared_ptr<AuthInfo>);//对方同意好友申请，认证到好友
    void sig_auth_rsp(std::shared_ptr<AuthRsp>);//同意对方的好友申请
};


#endif // TCPMGR_H

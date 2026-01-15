#ifndef TCPMGR_H
#define TCPMGR_H

#include <QTcpSocket>
#include "singleton.h"
#include "global.h"

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
    void slot_send_data(ReqId reqId, QString data);//发送数据
signals:
    void sig_con_success(bool bsuccess);//连接成功信号
    void sig_send_data(ReqId reqId, QString data);//发送数据信号
    void sig_swich_chatdlg();//切换进聊天窗口信号
    void sig_login_failed(int);//登录失败信号
};


#endif // TCPMGR_H

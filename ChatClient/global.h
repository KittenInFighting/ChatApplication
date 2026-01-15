#ifndef GLOBAL_H
#define GLOBAL_H

//刷新qss
#include <QWidget>
#include <functional>
#include "QStyle"
#include <QRegularExpression>
#include <memory>
#include <iostream>
#include <mutex>
#include <QByteArray>
#include <QNetworkReply>
#include <QJsonObject>
#include <QDir>
#include <QSettings>

extern std::function<void(QWidget *)> repolish;

enum ReqId{
    ID_GET_VARIFY_CODE = 1001, // 获取验证码
    ID_REG_USER = 1002, //注册用户
    ID_RESET_PWD = 1003, //重置密码
    ID_LOGIN_USER = 1004, //用户登录
    ID_CHAT_LOGIN = 1005, //连接聊天服务器
};

enum Modules{
    REGISTERMOD = 0,
    RESETMOD = 1,
    LOGINMOD = 2,
};

enum TipErr{
    TIP_SUCCESS = 0,
    TIP_EMAIL_ERR = 1,
    TIP_PWD_ERR = 2,
    TIP_VERIFY_ERR = 3,
    TIP_USER_ERR = 4,
};

enum ErrorCodes{
    SUCCESS = 0,
    ERR_JSON = 1,   //json解析失败
    ERR_NETWORK = 2,//网络错误
};


struct ServerInfo{//用于tcp通信
    QString Host;
    QString Port;
    QString Token;
    int Uid;
};
extern QString gate_url_prefix;
#endif // GLOBAL_H

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
    ID_CHAT_LOGIN_RSP= 1006, //解析登陆聊天服务器回包
    ID_SEARCH_USER_REQ = 1007, //用户搜索请求
    ID_SEARCH_USER_RSP = 1008, //搜索用户回包
    ID_ADD_FRIEND_REQ = 1009,  //添加好友申请
    ID_ADD_FRIEND_RSP = 1010, //申请添加好友回复
    ID_NOTIFY_ADD_FRIEND_REQ = 1011,  //通知用户添加好友申请
    ID_AUTH_FRIEND_REQ = 1013,  //认证好友请求
    ID_AUTH_FRIEND_RSP = 1014,  //认证好友回复
    ID_NOTIFY_AUTH_FRIEND_REQ = 1015, //通知用户认证好友申请
    ID_TEXT_CHAT_MSG_REQ  = 1017,  //文本聊天信息请求
    ID_TEXT_CHAT_MSG_RSP  = 1018,  //文本聊天信息回复
    ID_NOTIFY_TEXT_CHAT_MSG_REQ = 1019, //通知用户文本聊天信息
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

enum ChatUIMode{
    SearchMode,
    ChatMode,
    ContactMode,
};

enum ListItemType{
    CHAT_USER_ITEM,//聊天item
    CONTACT_USER_ITEM,//联系人item
    SEARCH_USER_ITEM,//搜索到的用户item
    ADD_USER_ITEM,//用于提示添加用户item
    INVALID_ITEM,//不可点击item
    GROUP_TIP_ITEM,//分组栏提示item
    APPLY_FRIEND_ITEM, //好友信息条申请item
};
enum class ChatRole
{
    Self,
    Other
};

struct MsgInfo{
    QString msgFlag;//"text,image,file"
    QString content;//表示文件和图像的url,文本信息
    QPixmap pixmap;//文件和图片的缩略图
};

enum ClickLbState{
    Normal = 0,
    Selected = 1
};

const std::vector<QString>  strs ={"hello !00000000000000",
                             "nice to meet u0000000000000",
                             "Ohiyo0000000000000000",
                             "Dear00000000000000",
                             "My Honey00000000000000",
                             "hello !",
                             "nice to meet u",
                             "Ohiyo",
                             "My Honey"
                             "Dear"};

const std::vector<QString> heads = {
    ":/res/1.jpg",
};

const std::vector<QString> names = {
    "Mihariooooooooo",
    "Mahiro0000000000",
    "Asahi00000000000",
    "Momiji000000000",
    "Kade00000000000",
    "Mihari",
    "Mahiro"
    "Asahi"
    "Momiji"
    "Kade"
};

const std::vector<QString> times = {
    "2025/06/08",
    "9:30",
    "8:00",
    "7:30",
    "13:00",
};

//申请好友标签输入框最低长度
const int MIN_APPLY_LABEL_ED_LEN = 40;

extern QString gate_url_prefix;
const int CHAT_COUNT_PER_PAGE = 13;
#endif // GLOBAL_H

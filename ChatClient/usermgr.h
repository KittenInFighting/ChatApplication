#ifndef USERMGR_H
#define USERMGR_H

#include "userdata.h"
#include <QObject>
#include <vector>
#include <memory>
#include <singleton.h>

class UserMgr:public QObject,public Singleton<UserMgr>,
                public std::enable_shared_from_this<UserMgr>
{
    Q_OBJECT
public:
    friend class Singleton<UserMgr>;
    ~ UserMgr();
    void SetUserInfo(std::shared_ptr<UserInfo> user_info);
    void SetToken(QString token);
    int GetUid();
    QString GetName();
    QString GetNick();
    QString GetIcon();
    QString GetDesc();
    auto GetApplyList()->std::vector<std::shared_ptr<ApplyInfo>>;
    std::shared_ptr<UserInfo> GetUserInfo();
private:
    UserMgr();
    std::shared_ptr<UserInfo> _user_info = nullptr;
    std::vector<std::shared_ptr<ApplyInfo>> _apply_list;
    QString _name;
    QString _token;
    int _uid;
public slots:
};
#endif // USERMGR_H

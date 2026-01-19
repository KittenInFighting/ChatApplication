#include "usermgr.h"

UserMgr::UserMgr() {

}

void UserMgr::SetToken(QString token)
{
    _token = token;
}

void UserMgr::SetUserInfo(std::shared_ptr<UserInfo> user_info) {
    _user_info = user_info;
}

QString UserMgr::GetName()
{
    return _user_info->_name;
}

QString UserMgr::GetNick()
{
    return _user_info->_nick;
}

QString UserMgr::GetIcon()
{
    return _user_info->_icon;
}

QString UserMgr::GetDesc()
{
    return _user_info->_desc;
}

auto UserMgr::GetApplyList() -> std::vector<std::shared_ptr<ApplyInfo> >
{
    return _apply_list;
}

std::shared_ptr<UserInfo> UserMgr::GetUserInfo()
{
    return _user_info;
}

UserMgr::~UserMgr()
{

}

#pragma once
#include <functional>

enum ErrorCodes {
  Success = 0,
  Error_Json = 1001,//Json解析错误
  RPCFailed = 1002,//RPC请求错误
  VerifyExpired = 1003,//验证码过期
  VerifyCodeErr = 1004,//验证码错误
  UserExist = 1005,//用户已存在
  PasswdErr = 1006,//密码错误
  EmailNotMatch = 1007,//邮箱不匹配
  PasswdUpFailed =1008,//密码更换失败
  PasswdInvalid =1009,//密码不符合
  TokenInvalid = 1010,   //Token失效
  UidInvalid = 1011,  //uid无效
};

//RAII机制的Defer类，用于执行函数，退出作用域时自动释放函数获得的资源
class Defer 
{
public:
  // 接受一个lambda表达式或者函数指针
  Defer(std::function<void()> func) : func_(func) {}

  // 析构函数中执行传入的函数
  ~Defer() {
	func_();
  }

private:
  std::function<void()> func_;
};

#define USERIPPREFIX  "uip_"
#define USERTOKENPREFIX  "utoken_"
#define IPCOUNTPREFIX  "ipcount_"
#define USER_BASE_INFO "ubaseinfo_"
#define LOGIN_COUNT  "logincount"
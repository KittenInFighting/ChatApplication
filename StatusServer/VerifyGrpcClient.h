#pragma once
#include "Const.h"
#include "ConfigMgr.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <grpcpp/grpcpp.h>
#include <condition_variable>

class RPConPool {
public:
  RPConPool(std::size_t poolsize, std::string host, std::string port);
  ~RPConPool();
  void Close();
  void returnConnection(std::unique_ptr<message::VerifyService::Stub> context);
  auto getConnection() -> std::unique_ptr<message::VerifyService::Stub>;

private:
  std::atomic<bool> _b_stop;
  std::size_t _poolsize;
  std::string _host;
  std::string _port;
  std::mutex _mutex;
  std::condition_variable _cond;
  //ø…”≈ªØ
  std::queue<std::unique_ptr<message::VerifyService::Stub>> _connections;
};

class VerifyGrpcClient:public Singleton<VerifyGrpcClient> 
{

  friend class Singleton<VerifyGrpcClient>;

public:
  message::GetVerifyRsp GetVerifyCode(std::string email);

private:
  std::unique_ptr<RPConPool> _pool;
  VerifyGrpcClient();
};


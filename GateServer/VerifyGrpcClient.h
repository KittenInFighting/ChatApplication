#pragma once
#include <grpcpp/grpcpp.h>
#include "Const.h"
#include "Singleton.h"
#include "message.grpc.pb.h"
#include <atomic>
#include <queue>
#include <mutex>
#include <condition_variable>
#include "ConfigMgr.h"
//using grpc::Channel;
//using grpc::Status;
//using grpc::ClientContext;
//
//using message::GetVarifyReq;
//using message::GetVarifyRsp;
//using message::VarifyService;

class RPConPool {
public:
	RPConPool(std::size_t poolsize, std::string host, std::string port):_poolsize(poolsize),_host(host),_port(port)
	{
		for (std::size_t i = 0; i < _poolsize; i++) {
			std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port,
				grpc::InsecureChannelCredentials());
			_connections.push(message::VerifyService::NewStub(channel));
		}
	}
	~RPConPool() {
		std::lock_guard<std::mutex> lock(_mutex);
		Close();
		while (!_connections.empty()) {
			_connections.pop();
		}
	}

	void Close()
	{
		_b_stop = true;
		_cond.notify_all();
	}

	std::unique_ptr<message::VerifyService::Stub> getConnection() {
		std::unique_lock<std::mutex> lock(_mutex);
		_cond.wait(lock, [this]() {
			if (this -> _b_stop) {
				return true;
			}
			return !_connections.empty();});

			if (_b_stop) {
				return nullptr;
			}
			auto context = std::move(_connections.front());
			_connections.pop();
			return context;
	}

	void returnConnection(std::unique_ptr<message::VerifyService::Stub> context) {
		std::lock_guard<std::mutex> lco(_mutex);
		if (_b_stop) {
			return;
		}

		_connections.push(std::move(context));
		_cond.notify_one();
	}
private:
	std::atomic<bool> _b_stop;
	std::size_t _poolsize;
	std::string _host;
	std::string _port;
	//ø…”≈ªØ
	std::queue<std::unique_ptr<message::VerifyService::Stub>> _connections;
	std::condition_variable _cond;
	std::mutex _mutex;
};

class VerifyGrpcClient:public Singleton<VerifyGrpcClient> 
{
	friend class Singleton<VerifyGrpcClient>;
public:
	message::GetVerifyRsp GetVerifyCode(std::string email) {
		grpc::ClientContext  context;
		message::GetVerifyRsp reply;
		message::GetVerifyReq request;
		request.set_email(email);
		auto stub = _pool->getConnection();
		grpc::Status status = stub -> GetVerifyCode(&context, request, &reply);
		if (status.ok()) {
			_pool->returnConnection(std::move(stub));
			return reply;
		}
		else {
			_pool->returnConnection(std::move(stub));
			reply.set_error(ErrorCodes::RPCFailed);
			return reply;
		}


	}

private:
	std::unique_ptr<RPConPool> _pool;
	VerifyGrpcClient() {
		auto& gCfgMgr = ConfigMgr::Inst();
		std::string host = gCfgMgr["VerifyServer"]["Host"];
		std::string port = gCfgMgr["VerifyServer"]["Port"];

		_pool.reset(new RPConPool(5, host, port));
	}
};


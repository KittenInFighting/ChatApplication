#include "VerifyGrpcClient.h"

RPConPool::RPConPool(std::size_t poolsize, std::string host, std::string port) :_poolsize(poolsize), _host(host), _port(port){
	for (std::size_t i = 0; i < _poolsize; i++) {
		std::shared_ptr<grpc::Channel> channel = grpc::CreateChannel(host + ":" + port,
			grpc::InsecureChannelCredentials());
		_connections.push(message::VerifyService::NewStub(channel));
	}
}

RPConPool::~RPConPool() {
	std::lock_guard<std::mutex> lock(_mutex);
	Close();
	while (!_connections.empty()) {
		_connections.pop();
	}
}

void RPConPool::Close() {
	_b_stop = true;
	_cond.notify_all();
}

auto RPConPool::getConnection() -> std::unique_ptr<message::VerifyService::Stub> {
	std::unique_lock<std::mutex> lock(_mutex);
	_cond.wait(lock, [this]() {
		if (this->_b_stop) {
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

void RPConPool::returnConnection(std::unique_ptr<message::VerifyService::Stub> context) {
	std::lock_guard<std::mutex> lco(_mutex);
	if (_b_stop) {
		return;
	}

	_connections.push(std::move(context));
	_cond.notify_one();
}

VerifyGrpcClient::VerifyGrpcClient() {
	auto& gCfgMgr = ConfigMgr::Inst();
	std::string host = gCfgMgr["VerifyServer"]["Host"];
	std::string port = gCfgMgr["VerifyServer"]["Port"];

	_pool.reset(new RPConPool(5, host, port));
}

message::GetVerifyRsp VerifyGrpcClient::GetVerifyCode(std::string email) {
	grpc::ClientContext  context;
	message::GetVerifyRsp response;
	message::GetVerifyReq request;
	request.set_email(email);
	auto stub = _pool->getConnection();
	grpc::Status status = stub->GetVerifyCode(&context, request, &response);
	if (status.ok()) {
		_pool->returnConnection(std::move(stub));
		return response;
	}
	else {
		_pool->returnConnection(std::move(stub));
		response.set_error(ErrorCodes::RPCFailed);
		return response;
	}
}
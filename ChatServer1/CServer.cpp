#include "CServer.h"
#include "AsioIOServicePool.h"
#include "UserMgr.h"
#include "ConfigMgr.h"
#include "Const.h"
#include "RedisMgr.h"
#include <cstdlib>
#include <iostream>
#include <mutex>

namespace {
void UpdateLoginCount(const std::string& server_name, int delta) {
	if (server_name.empty() || delta == 0) {
		return;
	}
	auto rd_res = RedisMgr::GetInstance()->HGet(LOGIN_COUNT, server_name);
	int count = 0;
	if (!rd_res.empty()) {
		count = std::atoi(rd_res.c_str());
	}
	count += delta;
	if (count < 0) {
		count = 0;
	}
	RedisMgr::GetInstance()->HSet(LOGIN_COUNT, server_name, std::to_string(count));
}
}

CServer::CServer(boost::asio::io_context& io_context, short port) :_io_context(io_context), _port(port),
_acceptor(io_context, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port))
{
	_server_name = ConfigMgr::Inst().GetValue("SelfServer", "Name");
	std::cout << "Server start success, listen on port : " << _port << std::endl;
	StartAccept();
}

CServer::~CServer() {
	std::cout << "Server destruct listen on port : " << _port << std::endl;
}
void CServer::HandleAccept(std::shared_ptr<CSession> new_session, const boost::system::error_code& error) {
	if (!error) {
		new_session->Start();
		{
			std::lock_guard<std::mutex> lock(_mutex);
			_sessions.insert(std::make_pair(new_session->GetSessionId(), new_session));
		}
		UpdateLoginCount(_server_name, 1);
	}
	else {
		std::cout << "session accept failed, error is " << error.what() << std::endl;
	}

	StartAccept();
}

void CServer::StartAccept() {
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<CSession> new_session = std::make_shared<CSession>(io_context, this);
	_acceptor.async_accept(new_session->GetSocket(), std::bind(&CServer::HandleAccept, this, new_session, std::placeholders::_1));
}

void CServer::ClearSession(std::string uuid) {
	int uid = 0;
	bool existed = false;
	{
		std::lock_guard<std::mutex> lock(_mutex);
		auto iter = _sessions.find(uuid);
		if (iter != _sessions.end()) {
			uid = iter->second->GetUserId();
			_sessions.erase(iter);
			existed = true;
		}
	}

	if (existed) {
		UpdateLoginCount(_server_name, -1);
		if (uid > 0) {
			UserMgr::GetInstance()->RmvUserSession(uid);
		}
	}
}

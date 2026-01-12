#pragma once
#include "Const.h"


class CServer:public std::enable_shared_from_this<CServer>
{
public:
	CServer(boost::asio::io_context& ioc, unsigned short& port);
	auto Start() -> void;

private:
	tcp::acceptor _acceptor;
	boost::asio::io_context& _ioc;
	tcp::socket _socket;
};


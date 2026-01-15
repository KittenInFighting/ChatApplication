#include "CServer.h"
#include "HttpConnection.h"
#include "AsioIOServicePool.h"

CServer::CServer(boost::asio::io_context& ioc, unsigned short& port):_ioc(ioc), _acceptor(ioc, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), port)), _socket(ioc){

}

void CServer::Start(){
	auto self = shared_from_this();
	auto& io_context = AsioIOServicePool::GetInstance()->GetIOService();
	std::shared_ptr<HttpConnection> new_con = std::make_shared<HttpConnection>(io_context);
	_acceptor.async_accept(new_con -> GetSocket(), [self, new_con](boost::system::error_code ec) {
		try {
			//出错放弃当前socket
			if (ec) {
				self -> Start();
				return;
			}

			//创建新连接，并创建HttpConnection类管理该连接
			new_con->Start();

			//继续监听
			self->Start();
		}
		catch (std::exception& exp)
		{
			std::cout << "exception is " << exp.what() << std::endl;
			self->Start();
		}
	});
}
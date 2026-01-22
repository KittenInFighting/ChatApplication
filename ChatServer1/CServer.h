#pragma once
#include <boost/asio.hpp>
#include "CSession.h"
#include <memory>
#include <map>
#include <mutex>
#include <string>
#include <boost/asio.hpp>


class CServer :public std::enable_shared_from_this<CServer>
{
public:
  CServer(boost::asio::io_context& io_context, short port);
  ~CServer();
  void ClearSession(std::string);
private:
  void HandleAccept(std::shared_ptr<CSession>, const boost::system::error_code& error);
  void StartAccept();
  boost::asio::io_context& _io_context;
  short _port;
  std::string _server_name;
  boost::asio::ip::tcp::acceptor _acceptor;
  std::map<std::string, std::shared_ptr<CSession>> _sessions;
  std::mutex _mutex;
};


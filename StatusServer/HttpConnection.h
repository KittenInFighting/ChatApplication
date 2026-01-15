#pragma once
#include <boost/beast.hpp>

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
  friend class LogicalSystem;
  HttpConnection(boost::asio::io_context& ioc);
  void Start();
  auto GetSocket() -> boost::asio::ip::tcp::socket&;

private:
  void CheckDeadline();
  void WriteResponse();
  void HandleReq();
  void PreParseGetParam();
  boost::asio::ip::tcp::socket _socket;
  boost::beast::flat_buffer _buffer{ 8192 };
  std::string _get_url;
  std::unordered_map<std::string, std::string> _get_params;
  boost::beast::http::request<boost::beast::http::dynamic_body> _request;
  boost::beast::http::response<boost::beast::http::dynamic_body> _response;
  boost::asio::steady_timer deadline_{
	_socket.get_executor(),std::chrono::seconds(60)
  };
	
};


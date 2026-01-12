#pragma once
#include "Const.h"

class HttpConnection:public std::enable_shared_from_this<HttpConnection>
{
public:
	friend class LogicalSystem;
	HttpConnection(boost::asio::io_context& ioc);
	auto Start() -> void;
	auto GetSocket() -> boost::asio::ip::tcp::socket&;

private:
	auto CheckDeadline() -> void;
	auto WriteResponse() -> void;
	auto HandleReq() -> void;
	auto PreParseGetParam() -> void;
	

	tcp::socket _socket;
	boost::beast::flat_buffer _buffer{ 8192 };
	std::string _get_url;
	std::unordered_map<std::string, std::string> _get_params;
	boost::beast::http::request<boost::beast::http::dynamic_body> _request;
	boost::beast::http::response<boost::beast::http::dynamic_body> _response;
	boost::asio::steady_timer deadline_{
		_socket.get_executor(),std::chrono::seconds(60)
	};
	
};


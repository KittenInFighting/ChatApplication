#pragma once
#include "Const.h"

class HttpConnection;
using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicalSystem:public Singleton<LogicalSystem>
{
public:
	friend class Singleton<LogicalSystem>;

	~LogicalSystem();
	auto RegGet(std::string url, HttpHandler handler) -> void;
	auto RegPost(std::string url, HttpHandler handler) -> void;
	auto HandleGet(std::string, std::shared_ptr<HttpConnection>) -> bool;
	auto HandlePost(std::string path, std::shared_ptr<HttpConnection> con) -> bool;

private:
	LogicalSystem();
	std::map<std::string, HttpHandler> _post_handlers;
	std::map<std::string, HttpHandler> _get_handlers;
};


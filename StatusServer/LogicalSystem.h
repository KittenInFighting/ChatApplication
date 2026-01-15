#pragma once
#include "Singleton.h"
#include <map>
#include <functional>

class HttpConnection;
using HttpHandler = std::function<void(std::shared_ptr<HttpConnection>)>;

class LogicalSystem:public Singleton<LogicalSystem>{
public:
  friend class Singleton<LogicalSystem>;

  ~LogicalSystem();
  void RegGet(std::string url, HttpHandler handler);
  void RegPost(std::string url, HttpHandler handler);
  bool HandleGet(std::string, std::shared_ptr<HttpConnection>);
  bool HandlePost(std::string path, std::shared_ptr<HttpConnection> con);

private:
  LogicalSystem();
  std::map<std::string, HttpHandler> _post_handlers;
  std::map<std::string, HttpHandler> _get_handlers;
};


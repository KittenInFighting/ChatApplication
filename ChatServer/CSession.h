#pragma once
#include "MsgNode.h"
#include "Const.h"
#include <iostream>
#include <memory>
#include <boost/asio.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <queue>

class CServer;
class CSession :public std::enable_shared_from_this<CSession>
{
public:
  CSession(boost::asio::io_context& io_context, CServer* server);
  ~CSession();
  void Start();
  void Send(char* msg, short max_length, short msgid);
  void Send(std::string msg, short msgid);
  auto GetSessionId() -> std::string&;
  auto GetSocket() -> boost::asio::ip::tcp::socket&;
  void Close();
  void AsyncReadHead(int total_len);
  void AsyncReadBody(int length);
  
private:
  void asyncReadFull(std::size_t maxLength, std::function<void(const boost::system::error_code&, std::size_t)> handler);
  void asyncReadLen(std::size_t  read_len, std::size_t total_len, 
	  std::function<void(const boost::system::error_code&, std::size_t)> handler);
  void HandleWrite(const boost::system::error_code& error);
  std::string _session_id;
  CServer* _server;
  boost::asio::ip::tcp::socket _socket;
  char _data[MAX_LENGTH];
  bool _b_close;
  std::queue<std::shared_ptr<SendNode> > _send_que;
  std::mutex _send_lock;
  //session 锁
  std::mutex _session_mtx;
  //收到的消息结构
  std::shared_ptr<RecvNode> _recv_msg_node;
  bool _b_head_parse;
  //收到的头部结构
  std::shared_ptr<MsgNode> _recv_head_node;
};

class LogicNode {
	friend class LogicSystem;
public:
	LogicNode(std::shared_ptr<CSession>, std::shared_ptr<RecvNode>);
private:
	std::shared_ptr<CSession> _session;
	std::shared_ptr<RecvNode> _recvnode;
};

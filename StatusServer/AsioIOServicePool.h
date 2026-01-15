#pragma once
#include "Singleton.h"
#include <vector>
#include <boost/asio.hpp>


class AsioIOServicePool :public Singleton<AsioIOServicePool>
{
  friend Singleton<AsioIOServicePool>;

public:
  using WorkGuard = boost::asio::executor_work_guard<boost::asio::io_context::executor_type>;
  ~AsioIOServicePool();
  AsioIOServicePool(const AsioIOServicePool&) = delete;
  AsioIOServicePool& operator=(const AsioIOServicePool&) = delete;	//禁止拷贝构造和赋值操作
  void Stop();
  auto GetIOService() -> boost::asio::io_context&; // 使用 round-robin轮询 的方式返回一个 io_service
	
private:
  AsioIOServicePool(std::size_t size = std::thread::hardware_concurrency());//根据机器创建线程数
  std::vector<boost::asio::io_context> _ioServices;
  std::vector<WorkGuard> _workGuards;//保持每个io_service一直运行
  std::vector<std::thread> _threads;
  std::size_t _nextIOService;
};
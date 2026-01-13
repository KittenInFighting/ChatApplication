#include "CServer.h"
#include "ConfigMgr.h"
#include "RedisMgr.h"
#include <iostream>
#include <cassert>
#include <sw/redis++/redis++.h>

int main()
{
    auto& gCfgMgr = ConfigMgr::Inst();
    std::string gate_port_str = gCfgMgr["GateServer"]["Port"];
    unsigned short gate_port = atoi(gate_port_str.c_str());
    try {
        unsigned short port = gate_port;
        boost::asio::io_context ioc{ 1 };
        boost::asio::signal_set signals(ioc, SIGINT, SIGTERM);
        signals.async_wait([&ioc](const boost::system::error_code& err, int signal_number) {
            if (err) return;
            ioc.stop();
            });

        std::make_shared<CServer>(ioc, port)->Start();
        std::cout << "Gate Server listen on port:" << port << std::endl;  
        ioc.run();
    }
    catch (std::exception& ec){
        std::cerr << "Error：" << ec.what() << std::endl;
        return EXIT_FAILURE;
    }
}
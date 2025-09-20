#include <boost/scope/defer.hpp >
#include <spdlog/spdlog.h>

import std.compat;
import socket.util;

int main()
{
    spdlog::set_pattern("[%C-%m-%d %T.%e] [%^%L%$] [t:%6t] [%-8!!:%4#] %v");

    int err = util::init();

    if (err != 0)
    {
        SPDLOG_ERROR("WSAStartup: {}", err);
        return 1;
    }

    BOOST_SCOPE_DEFER[]
    {
        util::deinit();
    };

    util::socket_t server = util::make_socket(util::af::inet, util::sock::stream, 0);
    if (server == util::invalid_socket)
    {
        SPDLOG_ERROR("socket: {}", util::last_error());
        return 1;
    }

    BOOST_SCOPE_DEFER[&]
    {
        util::close(server);
    };

    util::address addrServer = util::make_address(util::af::inet, 12345);

    if (!util::bind(server, addrServer))
    {
        SPDLOG_ERROR("bind: {}", util::last_error());
        return 1;
    }

    if (!util::listen(server, 5))
    {
        SPDLOG_ERROR("listen: {}", util::last_error());
        return 1;
    }

    int len;

    while (true)
    {
        util::address addrClient;
        util::socket_t client = util::accept(server, addrClient);

        if (client == util::invalid_socket)
        {
            SPDLOG_ERROR("accept: {}", util::last_error());
            return 1;
        }

        std::vector<char> info(128, '\0');
        sprintf_s(info.data(), info.size(), "ip: %s port: %d", util::inet_ntoa(addrClient.sin_addr),
                  addrClient.sin_port);
        SPDLOG_INFO("info: {}", info.data());

        BOOST_SCOPE_DEFER[&]
        {
            util::close(client);
        };

        std::string str = "Data from server!";
        len = util::send(client, str.c_str(), str.size(), 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == util::socket_error)
        {
            SPDLOG_ERROR("send: {}", util::last_error());
            return 1;
        }

        std::vector<char> recvData(1024, '\0');

        len = util::recv(client, recvData.data(), recvData.size(), 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == util::socket_error)
        {
            SPDLOG_ERROR("recv: {}", util::last_error());
            return 1;
        }

        SPDLOG_INFO("recv: {}", recvData.data());
    }
    return 0;
}

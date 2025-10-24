#pragma once

#include "client_decl.hpp"

#ifdef SOCKET_UTIL_HEADER_ONLY
#  include "socket_util/socket_util.hpp"
#else
import socket.util;
#endif // SOCKET_UTIL_HEADER_ONLY

import std.compat;

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

    util::socket_t client =
        util::make_socket(util::af::inet, util::sock::stream, 0);
    if (client == util::invalid_socket)
    {
        SPDLOG_ERROR("socket: {}", util::last_error());
        return 1;
    }

    BOOST_SCOPE_DEFER[&]
    {
        util::close(client);
    };

    util::address addr = util::make_address(util::af::inet, 12345, "127.0.0.1");

    if (!util::connect(client, addr))
    {
        SPDLOG_ERROR("connect: {}", util::last_error());
        return 1;
    }

    std::vector<char> recvData(1024, '\0');

    int len = util::recv(client, recvData.data(), recvData.size(), 0);
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

    std::string str = "Data from client!";

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

    return 0;
}

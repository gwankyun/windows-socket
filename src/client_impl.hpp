#pragma once

#include "client_decl.hpp"

#ifdef SOCKET_UTIL_HEADER_ONLY
#  include "socket_util/socket_util.hpp"
#else
import socket.util;
#endif // SOCKET_UTIL_HEADER_ONLY

import std.compat;

using namespace std::chrono_literals;

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

    auto use_async = true;

    if (use_async)
    {
        if (!util::connect(client, addr))
        {
            SPDLOG_ERROR("connect: {}", util::last_error());
            return 1;
        }
    }
    else
    {
        if (!util::set_nonblocking(client, true))
        {
            SPDLOG_ERROR("connect: {}", util::last_error());
            return 1;
        }

        BOOST_SCOPE_DEFER[&]{
            // util::set_nonblocking(client, false);
        };

        SPDLOG_INFO("connect begin");
        util::select_config config;
        config.timeval.tv_sec = 1;
        config.timeval.tv_usec = 0;
        int result = util::connect(client, addr, config);
        SPDLOG_INFO("connect end");
        if (result == 0)
        {
            SPDLOG_INFO("connect success");
        }
        else if (result == -1)
        {
            SPDLOG_ERROR("connect: {}", util::last_error());
            return 1;
        }
        else if (result == 1)
        {
            SPDLOG_ERROR("timeout");
            return 1;
        }
    }

    std::vector<char> recvData(1024, '\0');

    int len = -1;

    while (true)
    {
        std::size_t offset = 0;

        if (use_async)
        {
            util::select_config config;
            config.timeval.tv_sec = 0;
            config.timeval.tv_usec = 100;

            auto flag = true;

            while (flag)
            {
                auto result = util::readable(client, config);
                switch (result)
                {
                case util::select_status::success:
                    SPDLOG_INFO("success");
                    flag = false;
                    break;
                case util::select_status::timeout:
                    SPDLOG_INFO("timeout");
                    std::this_thread::sleep_for(100ms);
                    continue;
                    break;
                case util::select_status::select_error:
                    SPDLOG_ERROR("select_error");
                    return 1;
                    break;
                case util::select_status::socket_error:
                    SPDLOG_ERROR("socket_error");
                    return 1;
                    break;
                default:
                    return 1;
                    break;
                }
            }
        }

        int len = util::recv(client, recvData.data() + offset,
                             recvData.size() - offset, 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == util::socket_error)
        {
            SPDLOG_ERROR("recv: {}", util::last_error());
            return 1;
        }

        offset += len;

        if (recvData[offset - 1] == '.')
        {
            break;
        }
    }
    // util::set_nonblocking(client, false);

    SPDLOG_INFO("recv: {}", recvData.data());

    std::string str = "Data from client.";

    while (true)
    {
        std::size_t offset = 0;

        if (use_async)
        {
            util::select_config config;
            config.timeval.tv_sec = 0;
            config.timeval.tv_usec = 100;

            auto flag = true;

            while (flag)
            {
                auto result = util::writable(client, config);
                switch (result)
                {
                case util::select_status::success:
                    SPDLOG_INFO("success");
                    flag = false;
                    break;
                case util::select_status::timeout:
                    SPDLOG_INFO("timeout");
                    std::this_thread::sleep_for(100ms);
                    continue;
                    break;
                case util::select_status::select_error:
                    SPDLOG_ERROR("select_error");
                    return 1;
                    break;
                case util::select_status::socket_error:
                    SPDLOG_ERROR("socket_error");
                    return 1;
                    break;
                default:
                    break;
                }
            }
        }

        len = util::send(client, str.c_str() + offset, str.size() - offset, 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == util::socket_error)
        {
            SPDLOG_ERROR("send: {}", util::last_error());
            return 1;
        }
        offset += len;
        if (offset = str.size() - 1)
        {
            break;
        }
    }

    return 0;
}

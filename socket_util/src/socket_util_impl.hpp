#pragma once

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
import winapi;
import socket.util.base;
import std;
#else
#  include "socket_util_base_impl.hpp"
#  include "winapi_impl.hpp"
#endif // SOCKET_UTIL_MODULE

#ifdef __INTELLISENSE__
#  include "socket_util_inc.h"
#endif

namespace util
{
    SOCKET_UTIL_INLINE int init()
    {
        winapi::WORD wVersionRequested = winapi::make_word(2, 2);
        winapi::WSADATA lpWSAData;

        return winapi::WSAStartup(wVersionRequested, &lpWSAData);
    }

    SOCKET_UTIL_INLINE bool deinit()
    {
        return winapi::WSACleanup() == 0;
    }

    SOCKET_UTIL_INLINE socket_t make_socket(af_type _af = af::inet,
                                            sock_type _type = sock::stream,
                                            int _protocol = 0)
    {
        return winapi::socket(static_cast<int>(_af), static_cast<int>(_type),
                              _protocol);
    }

    SOCKET_UTIL_INLINE bool close(socket_t _socket)
    {
        return winapi::closesocket(_socket) == 0;
    }

    SOCKET_UTIL_INLINE bool bind(socket_t _server, address& _addrServer)
    {
        using winapi::SOCKADDR;
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = winapi::bind(_server, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    SOCKET_UTIL_INLINE socket_t accept(socket_t _server, address& _addrServer)
    {
        using winapi::SOCKADDR;
        using winapi::SOCKADDR_IN;
        SOCKADDR_IN addrClient;
        int addrlen = sizeof(SOCKADDR);
        SOCKADDR* adder = reinterpret_cast<SOCKADDR*>(&_addrServer);
        socket_t client = winapi::accept(_server, adder, &addrlen);
        return client;
    }

    SOCKET_UTIL_INLINE bool connect(socket_t _client, address& _addrServer)
    {
        using winapi::SOCKADDR;
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = winapi::connect(_client, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    SOCKET_UTIL_INLINE int connect(socket_t _sock, address& _addr,
                                   select_config& _config)
    {
        // 尝试连接
        using winapi::SOCKADDR;
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addr);
        int result = winapi::connect(_sock, addr, sizeof(SOCKADDR));

        // 如果立即成功，直接返回
        if (result == 0)
        {
            return 0;
        }

        // 检查错误码是否为WSAEWOULDBLOCK，这是非阻塞模式下正在连接的正常返回
        if (winapi::WSAGetLastError() != winapi::macro::wsa_ewouldblock)
        {
            return -1; // 连接发生其他错误
        }

        // 使用select等待连接完成
        winapi::fd::zero(_config.write);
        winapi::fd::zero(_config.except);
        winapi::fd::set(_sock, _config.write);
        winapi::fd::set(_sock, _config.except);

        // 等待套接字变为可写（连接完成）或发生异常
        result = winapi::select(0, NULL, &_config.write, &_config.except,
                                &_config.timeval);

        if (result == 0)
        {
            // 超时
            return 1;
        }
        else if (result == winapi::macro::socket_error)
        {
            // select出错
            return -1;
        }
        else
        {
            // 检查是否是异常情况
            if (winapi::fd::isset(_sock, _config.except))
            {
                return -1;
            }

            // 检查连接是否成功
            if (winapi::fd::isset(_sock, _config.write))
            {
                // 连接完成，需要再次检查套接字错误状态
                int error = 0;
                int len = sizeof(error);
                if (winapi::getsockopt(_sock, winapi::macro::sol_socket,
                                       winapi::macro::so_error, (char*)&error,
                                       &len) == winapi::macro::socket_error)
                {
                    return -1;
                }

                if (error != 0)
                {
                    winapi::WSASetLastError(error);
                    return -1;
                }

                // 连接成功
                return 0;
            }
        }

        return -1;
    }

    SOCKET_UTIL_INLINE bool listen(socket_t& _socket, int _backlog)
    {
        return winapi::listen(_socket, _backlog) == 0;
    }

    SOCKET_UTIL_INLINE char* inet_ntoa(winapi::in_addr _in)
    {
        return winapi::inet_ntoa(_in);
    }

    SOCKET_UTIL_INLINE int last_error()
    {
        return winapi::WSAGetLastError();
    }

    SOCKET_UTIL_INLINE address make_address(af_type _af, uint16_t _port,
                                            std::string _ip)
    {
        using winapi::SOCKADDR_IN;
        SOCKADDR_IN addr;
        addr.sin_family = static_cast<int>(_af);
        if (_ip.empty())
        {
            addr.sin_addr.s_addr = winapi::htonl(winapi::macro::inaddr_any);
        }
        else
        {
            addr.sin_addr.s_addr = winapi::inet_addr(_ip.c_str());
        }
        addr.sin_port = winapi::htons(_port);
        return addr;
    }

    SOCKET_UTIL_INLINE int send(socket_t _s, const char* _data,
                                std::size_t _len, int _flags)
    {
        return winapi::send(_s, _data, static_cast<int>(_len), _flags);
    }

    SOCKET_UTIL_INLINE select_status_type writable(socket_t sock,
                                                   select_config& _config)
    {
        winapi::fd::zero(_config.write);
        winapi::fd::zero(_config.except);
        winapi::fd::set(sock, _config.write);
        winapi::fd::set(sock, _config.except);

        int result = winapi::select(0, NULL, &_config.write, &_config.except,
                                    &_config.timeval);

        if (result > 0)
        {
            if (winapi::fd::isset(sock, _config.write))
            {
                return select_status::success; // 可以发送数据
            }
            else if (winapi::fd::isset(sock, _config.except))
            {
                return select_status::socket_error;
            }
        }
        else if (result == 0)
        {
            return select_status::timeout;
        }
        else
        {
            return select_status::select_error;
        }

        return select_status::unknown_error;
    }

    SOCKET_UTIL_INLINE select_status_type readable(socket_t sock,
                                                   select_config& _config)
    {
        winapi::fd::zero(_config.read);
        winapi::fd::zero(_config.except);
        winapi::fd::set(sock, _config.read);
        winapi::fd::set(sock, _config.except);

        int result = winapi::select(0, &_config.read, NULL, &_config.except,
                                    &_config.timeval);

        if (result > 0)
        {
            if (winapi::fd::isset(sock, _config.read))
            {
                return select_status::success; // 可以发送数据
            }
            else if (winapi::fd::isset(sock, _config.except))
            {
                return select_status::socket_error;
            }
        }
        else if (result == 0)
        {
            return select_status::timeout;
        }
        else
        {
            return select_status::select_error;
        }

        return select_status::unknown_error;
    }

    SOCKET_UTIL_INLINE int recv(socket_t _s, char* _data, std::size_t _len,
                                int _flags)
    {
        return winapi::recv(_s, _data, static_cast<int>(_len), _flags);
    }

    SOCKET_UTIL_INLINE uint16_t ntohs(uint16_t _ns)
    {
        return winapi::ntohs(_ns);
    }

    SOCKET_UTIL_INLINE uint32_t ntohl(uint32_t _nl)
    {
        return winapi::ntohl(_nl);
    }

    SOCKET_UTIL_INLINE uint16_t htons(uint16_t _hs)
    {
        return winapi::htons(_hs);
    }

    SOCKET_UTIL_INLINE uint32_t htonl(uint32_t _hl)
    {
        return winapi::htonl(_hl);
    }

    SOCKET_UTIL_INLINE bool set_nonblocking(socket_t _sock, bool _nonblocking)
    {
        using winapi::macro;
        winapi::u_long mode = _nonblocking ? 1 : 0;
        int result = winapi::ioctlsocket(_sock, macro::fionbio, &mode);
        return result == macro::no_error;
    }
} // namespace util

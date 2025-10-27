#pragma once

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
import winapi;
import socket.util.base;
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
} // namespace util

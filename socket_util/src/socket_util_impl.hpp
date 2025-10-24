#pragma once

#ifndef SOCKET_UTIL_MODULE
#  include "socket_util_inner.h"
#endif // !SOCKET_UTIL_MODULE

#include "macro.h"

namespace util
{
    SOCKET_UTIL_INLINE int init()
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA lpWSAData;

        return ::WSAStartup(wVersionRequested, &lpWSAData);
    }

    SOCKET_UTIL_INLINE bool deinit()
    {
        return ::WSACleanup() == 0;
    }

    SOCKET_UTIL_INLINE socket_t make_socket(af_type _af = af::inet,
                                            sock_type _type = sock::stream,
                                            int _protocol = 0)
    {
        return ::socket(static_cast<int>(_af), static_cast<int>(_type),
                        _protocol);
    }

    SOCKET_UTIL_INLINE bool close(socket_t _socket)
    {
        return ::closesocket(_socket) == 0;
    }

    SOCKET_UTIL_INLINE bool bind(socket_t _server, address& _addrServer)
    {
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = ::bind(_server, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    SOCKET_UTIL_INLINE socket_t accept(socket_t _server, address& _addrServer)
    {
        SOCKADDR_IN addrClient;
        int addrlen = sizeof(SOCKADDR);
        SOCKADDR* adder = reinterpret_cast<SOCKADDR*>(&_addrServer);
        socket_t client = ::accept(_server, adder, &addrlen);
        return client;
    }

    SOCKET_UTIL_INLINE bool connect(socket_t _client, address& _addrServer)
    {
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = ::connect(_client, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    SOCKET_UTIL_INLINE bool listen(socket_t& _socket, int _backlog)
    {
        return ::listen(_socket, _backlog) == 0;
    }

    SOCKET_UTIL_INLINE char* inet_ntoa(in_addr _in)
    {
        return ::inet_ntoa(_in);
    }

    SOCKET_UTIL_INLINE int last_error()
    {
        return ::WSAGetLastError();
    }

    SOCKET_UTIL_INLINE address make_address(af_type _af, uint16_t _port,
                                            std::string _ip)
    {
        SOCKADDR_IN addr;
        addr.sin_family = static_cast<int>(_af);
        if (_ip.empty())
        {
            addr.sin_addr.s_addr = ::htonl(INADDR_ANY);
        }
        else
        {
            addr.sin_addr.s_addr = ::inet_addr(_ip.c_str());
        }
        addr.sin_port = ::htons(_port);
        return addr;
    }

    SOCKET_UTIL_INLINE int send(socket_t _s, const char* _data,
                                std::size_t _len, int _flags)
    {
        return ::send(_s, _data, static_cast<int>(_len), _flags);
    }

    SOCKET_UTIL_INLINE int recv(socket_t _s, char* _data, std::size_t _len,
                                int _flags)
    {
        return ::recv(_s, _data, static_cast<int>(_len), _flags);
    }

    SOCKET_UTIL_INLINE uint16_t ntohs(uint16_t _ns)
    {
        return ::ntohs(_ns);
    }

    SOCKET_UTIL_INLINE uint32_t ntohl(uint32_t _nl)
    {
        return ::ntohl(_nl);
    }

    SOCKET_UTIL_INLINE uint16_t htons(uint16_t _hs)
    {
        return ::htons(_hs);
    }

    SOCKET_UTIL_INLINE uint32_t htonl(uint32_t _hl)
    {
        return ::htonl(_hl);
    }
} // namespace util

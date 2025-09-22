#pragma once

#ifndef SOCKET_UTIL_MODULE
#  include "socket_util.h"
#endif // !SOCKET_UTIL_MODULE

namespace util
{
    int init()
    {
        WORD wVersionRequested = MAKEWORD(2, 2);
        WSADATA lpWSAData;

        return ::WSAStartup(wVersionRequested, &lpWSAData);
    }

    bool deinit()
    {
        return ::WSACleanup() == 0;
    }

    socket_t make_socket(af_type _af = af::inet, sock_type _type = sock::stream, int _protocol = 0)
    {
        return ::socket(static_cast<int>(_af), static_cast<int>(_type), _protocol);
    }

    bool close(socket_t _socket)
    {
        return ::closesocket(_socket) == 0;
    }

    bool bind(socket_t _server, address& _addrServer)
    {
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = ::bind(_server, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    socket_t accept(socket_t _server, address& _addrServer)
    {
        SOCKADDR_IN addrClient;
        int addrlen = sizeof(SOCKADDR);
        SOCKADDR* adder = reinterpret_cast<SOCKADDR*>(&_addrServer);
        socket_t client = ::accept(_server, adder, &addrlen);
        return client;
    }

    bool connect(socket_t _client, address& _addrServer)
    {
        SOCKADDR* addr = reinterpret_cast<SOCKADDR*>(&_addrServer);
        int err = ::connect(_client, addr, sizeof(SOCKADDR));
        return err == 0;
    }

    bool listen(socket_t& _socket, int _backlog)
    {
        return ::listen(_socket, _backlog) == 0;
    }

    char* inet_ntoa(in_addr _in)
    {
        return ::inet_ntoa(_in);
    }

    int last_error()
    {
        return ::WSAGetLastError();
    }

    address make_address(af_type _af, uint16_t _port, std::string _ip)
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

    int send(socket_t _s, const char* _data, std::size_t _len, int _flags)
    {
        return ::send(_s, _data, static_cast<int>(_len), _flags);
    }

    int recv(socket_t _s, char* _data, std::size_t _len, int _flags)
    {
        return ::recv(_s, _data, static_cast<int>(_len), _flags);
    }
} // namespace util

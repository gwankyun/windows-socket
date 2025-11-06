#pragma once

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
export import socket.util.base;
export import winapi;
#else
#  include "socket_util_base_impl.hpp"
#  include "winapi_impl.hpp"
#endif // SOCKET_UTIL_MODULE

SOCKET_UTIL_MODULE_EXPORT namespace util
{
    address make_address(af_type _af, uint16_t _port, std::string _ip = "");

    int init();

    bool deinit();

    bool listen(socket_t & _socket, int _backlog);

    socket_t make_socket(af_type _af, sock_type _type, int _protocol);

    bool close(socket_t _socket);

    bool bind(socket_t _server, address & _addrServer);

    socket_t accept(socket_t _server, address & _addrServer);

    bool connect(socket_t _client, address & _addrServer);

    int connect_with_select(socket_t _sock, address & _addr,
                            winapi::timeval & _timeout);

    int last_error();

    char* inet_ntoa(winapi::in_addr _in);

    int send(socket_t _s, const char* _data, std::size_t _len, int _flags = 0);

    int recv(socket_t _s, char* _data, std::size_t _len, int _flags = 0);

    uint16_t ntohs(uint16_t _ns);
    uint32_t ntohl(uint32_t _nl);
    uint16_t htons(uint16_t _hs);
    uint32_t htonl(uint32_t _hl);

    bool set_nonblocking(socket_t _sock, bool _nonblocking);
    select_status_type writable_with_select(socket_t sock,
                                            winapi::timeval & timeout);
    select_status_type readable_with_select(socket_t sock,
                                            winapi::timeval & timeout);
} // namespace util

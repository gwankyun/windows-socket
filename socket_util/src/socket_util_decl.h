#pragma once

#ifndef HAS_CSTDINT
#  define HAS_CSTDINT 1
#endif // !HAS_CSTDINT

#ifndef SOCKET_UTIL_MODULE
#  include "socket_util_inc.h"

#  if HAS_CSTDINT
#    include <cstdint>
#  endif
#  include <string>
#endif // !SOCKET_UTIL_MODULE

namespace util
{
#if HAS_CSTDINT
    typedef std::uint8_t uint8_t;
    typedef std::uint16_t uint16_t;
    typedef std::uint32_t uint32_t;
#else
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
#endif

    struct af
    {
        enum type
        {
            inet = AF_INET
        };
    };
    typedef af::type af_type;

    struct sock
    {
        enum type
        {
            stream = SOCK_STREAM
        };
    };
    typedef sock::type sock_type;

    enum socket_error
    {
        invalid_socket = INVALID_SOCKET
    };

    enum io_error
    {
        socket_error = SOCKET_ERROR
    };

    typedef SOCKET socket_t;

    typedef sockaddr_in address;

    address make_address(af_type _af, uint16_t _port, std::string _ip = "");

    int init();

    bool deinit();

    bool listen(socket_t& _socket, int _backlog);

    socket_t make_socket(af_type _af, sock_type _type, int _protocol);

    bool close(socket_t _socket);

    bool bind(socket_t _server, address& _addrServer);

    socket_t accept(socket_t _server, address& _addrServer);

    bool connect(socket_t _client, address& _addrServer);

    int last_error();

    char* inet_ntoa(in_addr _in);

    int send(socket_t _s, const char* _data, std::size_t _len, int _flags = 0);

    int recv(socket_t _s, char* _data, std::size_t _len, int _flags = 0);

    uint16_t ntohs(uint16_t _ns);
    uint32_t ntohl(uint32_t _nl);
    uint32_t htonl(uint32_t _hl);
} // namespace util

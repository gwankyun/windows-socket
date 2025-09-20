module;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>

export module socket.util;
import std;

export namespace util
{
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

    address make_address(af_type _af, std::uint16_t _port);
    address make_address(af_type _af, std::uint16_t _port, std::string _ip);

    int init();

    bool deinit();

    bool listen(socket_t& _socket, int _backlog);

    socket_t make_socket(af_type _af, sock_type _type, int _protocol);

    bool close(socket_t _socket);

    bool bind(socket_t server, address& addrServer);

    socket_t accept(socket_t server, address& addrServer);

    bool connect(socket_t client, address& addrServer);

    int last_error();

    char* inet_ntoa(in_addr _in);

    int send(socket_t _s, const char* _data, std::size_t _len, int _flags);

    int recv(socket_t _s, char* _data, std::size_t _len, int _flags);
} // namespace util

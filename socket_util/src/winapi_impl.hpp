#pragma once

#ifndef SOCKET_UTIL_MODULE
#  include "socket_util_inc.h"
#endif // !SOCKET_UTIL_MODULE

#ifdef __INTELLISENSE__
#  include "socket_util_inc.h"
#endif

#include "macro.h"

SOCKET_UTIL_MODULE_EXPORT namespace winapi
{
    using ::accept;
    using ::bind;
    using ::closesocket;
    using ::connect;
    using ::htonl;
    using ::htons;
    using ::inet_addr;
    using ::inet_ntoa;
    using ::ioctlsocket;
    using ::listen;
    using ::ntohl;
    using ::ntohs;
    using ::recv;
    using ::send;
    using ::socket;
    using ::WSACleanup;
    using ::WSAGetLastError;
    using ::WSAStartup;
    using ::select;
    using ::getsockopt;
    using ::WSASetLastError;

    using ::fd_set;
    using ::in_addr;
    using ::SOCKADDR;
    using ::SOCKADDR_IN;
    using ::SOCKET;
    using ::u_long;
    using ::WORD;
    using ::WSADATA;
    using ::timeval;

    SOCKET_UTIL_INLINE WORD make_word(unsigned char _a, unsigned char _b)
    {
        return MAKEWORD(_a, _b);
    }

    namespace fd
    {
        typedef fd_set type;

        SOCKET_UTIL_INLINE void zero(type& _set)
        {
            FD_ZERO(&_set);
        }

        SOCKET_UTIL_INLINE void set(SOCKET _socket, type& _set)
        {
            FD_SET(_socket, &_set);
        }

        SOCKET_UTIL_INLINE bool isset(SOCKET _socket, type& _set)
        {
            return FD_ISSET(_socket, &_set);
        }
    } // namespace fd

    struct macro
    {
        static const int inaddr_any = INADDR_ANY;
        static const int af_inet = AF_INET;
        static const int sock_stream = SOCK_STREAM;
        static const int invalid_socket = INVALID_SOCKET;
        static const int socket_error = SOCKET_ERROR;
        static const int fionbio = FIONBIO;
        static const int no_error = NO_ERROR;
        static const int wsa_ewouldblock = WSAEWOULDBLOCK;
        static const int sol_socket = SOL_SOCKET;
        static const int so_error = SO_ERROR;
    };
}

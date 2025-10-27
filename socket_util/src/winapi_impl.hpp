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
    using ::listen;
    using ::ntohl;
    using ::ntohs;
    using ::recv;
    using ::send;
    using ::socket;
    using ::WSACleanup;
    using ::WSAGetLastError;
    using ::WSAStartup;

    using ::WORD;
    using ::WSADATA;
    using ::SOCKADDR;
    using ::SOCKADDR_IN;
    using ::in_addr;
    using ::SOCKET;

    SOCKET_UTIL_INLINE WORD make_word(unsigned char _a, unsigned char _b)
    {
        return MAKEWORD(_a, _b);
    }

    struct macro
    {
        static const int inaddr_any = INADDR_ANY;
        static const int af_inet = AF_INET;
        static const int sock_stream = SOCK_STREAM;
        static const int invalid_socket = INVALID_SOCKET;
        static const int socket_error = SOCKET_ERROR;
    };
}

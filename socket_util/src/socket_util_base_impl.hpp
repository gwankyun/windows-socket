#pragma once

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
import winapi;
import std;
export import cstdint;
#else
#  include "cstdint_impl.hpp"
#  include "winapi_impl.hpp"
#  include <string>
#endif // SOCKET_UTIL_MODULE

SOCKET_UTIL_MODULE_EXPORT namespace util
{
    struct af
    {
        enum type
        {
            inet = winapi::macro::af_inet
        };
    };
    typedef af::type af_type;

    struct sock
    {
        enum type
        {
            stream = winapi::macro::sock_stream
        };
    };
    typedef sock::type sock_type;

    enum socket_error
    {
        invalid_socket = winapi::macro::invalid_socket
    };

    enum io_error
    {
        socket_error = winapi::macro::socket_error
    };

    typedef winapi::SOCKET socket_t;

    typedef winapi::SOCKADDR_IN address;

    struct select_status
    {
        enum type
        {
            success = 0,
            timeout,
            socket_error,
            select_error,
            unknown_error
        };
    };
    typedef select_status::type select_status_type;
} // namespace util

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

#include "macro.h"

SOCKET_UTIL_MODULE_EXPORT namespace util
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
} // namespace util

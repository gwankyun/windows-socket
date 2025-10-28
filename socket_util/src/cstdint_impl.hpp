#pragma once

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
import std;
#else
#  if SOCKET_UTIL_CXX >= SOCKET_UTIL_CXX_11
#    include <cstdint>
#  endif
#endif // SOCKET_UTIL_MODULE

SOCKET_UTIL_MODULE_EXPORT namespace util
{
#if SOCKET_UTIL_CXX >= SOCKET_UTIL_CXX_11
    using std::uint16_t;
    using std::uint32_t;
    using std::uint8_t;
#else
    typedef unsigned __int8 uint8_t;
    typedef unsigned __int16 uint16_t;
    typedef unsigned __int32 uint32_t;
#endif
}

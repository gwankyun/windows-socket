#pragma once

#ifndef HAS_CSTDINT
#  define HAS_CSTDINT 1
#endif // !HAS_CSTDINT

#include "macro.h"

#ifdef SOCKET_UTIL_MODULE
import std;
#else
#  if HAS_CSTDINT
#    include <cstdint>
#  endif
#endif // SOCKET_UTIL_MODULE

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
}

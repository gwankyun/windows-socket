module;

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#define SOCKET_UTIL_MODULE

export module socket.util;
import std;

export
{
#include "socket_util_decl.h"
}

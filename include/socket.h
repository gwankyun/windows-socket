#pragma once
#include <WinSock2.h>

class WSAGuard
{
public:
    WSAGuard()
    {
    }

    ~WSAGuard()
    {
        WSACleanup();
    }

private:

};

class SocketGuard
{
public:
    SocketGuard() : socket(INVALID_SOCKET)
    {
    }

    SocketGuard(SOCKET _socket) : socket(_socket)
    {
    }

    ~SocketGuard()
    {
        if (socket != INVALID_SOCKET)
        {
            closesocket(socket);
        }
    }

private:
    SOCKET socket;
};

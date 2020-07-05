#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <cstdio>
#include <iostream>
#include <string>
#include "ScopeGuard/ScopeGuard.hpp"

int main()
{
    int err;
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA lpWSAData;

    err = WSAStartup(wVersionRequested, &lpWSAData);

    if (err != 0)
    {
        std::cout << "WSAStartup: " << err << std::endl;
        return 1;
    }

    ON_SCOPE_EXIT(WSACleanup);

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        std::cout << "socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    ON_SCOPE_EXIT(closesocket, server);

    SOCKADDR_IN addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_port = htons(12345);

    err = bind(server, reinterpret_cast<SOCKADDR*>(&addrServer), sizeof(SOCKADDR));
    if (err != 0)
    {
        std::cout << "bind: " << WSAGetLastError() << std::endl;
        return 1;
    }

    err = listen(server, 5);
    if (err != 0)
    {
        std::cout << "listen: " << WSAGetLastError() << std::endl;
        return 1;
    }

    int len;

    while (true)
    {
        SOCKADDR_IN addrClient;
        int addrlen = sizeof(SOCKADDR);
        SOCKET client = accept(server, reinterpret_cast<SOCKADDR*>(&addrClient), &addrlen);

        if (client == INVALID_SOCKET)
        {
            std::cout << "accept: " << WSAGetLastError() << std::endl;
            return 1;
        }

        char info[128];
        memset(info, '\0', sizeof(info));
        sprintf_s(info, sizeof(info), "ip: %s port: %d", inet_ntoa(addrClient.sin_addr), addrClient.sin_port);
        std::cout << info << std::endl;

        ON_SCOPE_EXIT(closesocket, client);

        std::string str = "Data from server!";
        len = send(client, str.c_str(), static_cast<int>(str.size()), 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == SOCKET_ERROR)
        {
            std::cout << "send: " << WSAGetLastError() << std::endl;
        }

        char recvData[1024];
        memset(recvData, '\0', sizeof(recvData));

        len = recv(client, recvData, sizeof(recvData), 0);
        if (len == 0)
        {
            return 1;
        }
        else if (len == SOCKET_ERROR)
        {
            std::cout << "recv: " << WSAGetLastError() << std::endl;
            return 1;
        }

        std::cout << "recv: " << recvData << std::endl;
    }
    return 0;
}

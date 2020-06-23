// cpp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define _CRT_SECURE_NO_WARNINGS
#include <cstdio>
#include <iostream>
#include <string>
#include "socket.h"

#pragma comment(lib, "ws2_32.lib")

int main()
{
    int err;
    WORD wVersionRequested;
    WSADATA lpWSAData;

    wVersionRequested = MAKEWORD(2, 2);

    err = WSAStartup(wVersionRequested, &lpWSAData);

    if (err != 0)
    {
        std::cout << "WSAStartup: " << err << std::endl;
        return 1;
    }

    WSAGuard wsaGuard;

    SOCKET server = socket(AF_INET, SOCK_STREAM, 0);
    if (server == INVALID_SOCKET)
    {
        std::cout << "socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SocketGuard serverGuard(server);

    SOCKADDR_IN addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_port = htons(12345);

    err = bind(server, (SOCKADDR*)&addrServer, sizeof(SOCKADDR));
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
        SOCKET client = accept(server, (SOCKADDR*)&addrClient, &addrlen);

        if (client == INVALID_SOCKET)
        {
            std::cout << "accept: " << WSAGetLastError() << std::endl;
            return 1;
        }

        char info[128];
        memset(info, '\0', sizeof(info));

        sprintf(info, "ip: %s port: %d", inet_ntoa(addrClient.sin_addr), addrClient.sin_port);
        std::cout << info << std::endl;

        SocketGuard clientGuard(client);

        std::string str = "Data from server!";
        len = send(client, str.c_str(), (int)str.size(), 0);
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

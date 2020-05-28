// cpp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

//#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <iostream>
#include <WinSock2.h>
#include <string>

#pragma comment(lib, "ws2_32.lib")

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
    SocketGuard(): socket(INVALID_SOCKET)
    {
    }

    SocketGuard(SOCKET _socket): socket(_socket)
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

// cpp.cpp : 此文件包含 "main" 函数。程序执行将在此处开始并结束。
//

#define _WINSOCK_DEPRECATED_NO_WARNINGS
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

    WSAGuard wasGuard;

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET)
    {
        std::cout << "socket: " << WSAGetLastError() << std::endl;
        return 1;
    }

    SocketGuard clientGuard(client);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    err = connect(client, (SOCKADDR*)&addr, sizeof(SOCKADDR));

    if (err == INVALID_SOCKET)
    {
        std::cout << "connect: " << WSAGetLastError() << std::endl;
        return 1;
    }

    char recvData[1024];
    memset(recvData, '\0', sizeof(recvData));

    int len;

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

    std::string str = "Data from client!";

    len = send(client, str.c_str(), (int)str.size(), 0);
    if (len == 0)
    {
        return 1;
    }
    else if (len == SOCKET_ERROR)
    {
        std::cout << "send: " << WSAGetLastError() << std::endl;
        return 1;
    }

    return 0;
}

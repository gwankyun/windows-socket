#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
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
    std::cout << "WSAStartup" << std::endl;

    ON_SCOPE_EXIT(WSACleanup);

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET)
    {
        std::cout << "socket: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "socket" << std::endl;

    ON_SCOPE_EXIT(closesocket, client);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    err = connect(client, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR));

    if (err == INVALID_SOCKET)
    {
        std::cout << "connect: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "connect" << std::endl;

    int len;

    std::string str = "Data from client!";

    len = send(client, str.c_str(), static_cast<int>(str.size()), 0);
    std::cout << "send" << std::endl;
    std::cout << len << std::endl;
    if (len == 0)
    {
        return 1;
    }
    else if (len == SOCKET_ERROR)
    {
        std::cout << "send: " << WSAGetLastError() << std::endl;
        return 1;
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

    return 0;
}

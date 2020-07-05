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
        printf("[%s %d] WSAStartup faild:\n", __func__, __LINE__);
        return 1;
    }

    ON_SCOPE_EXIT(WSACleanup);

    SOCKET client = socket(AF_INET, SOCK_STREAM, 0);
    if (client == INVALID_SOCKET)
    {
        printf("[%s %d] socket error: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    ON_SCOPE_EXIT(closesocket, client);

    SOCKADDR_IN addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr("127.0.0.1");
    addr.sin_port = htons(12345);

    err = connect(client, reinterpret_cast<SOCKADDR*>(&addr), sizeof(SOCKADDR));

    if (err == INVALID_SOCKET)
    {
        printf("[%s %d] connect error: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    int len;

    std::string str = "Data from client!";

    len = send(client, str.c_str(), static_cast<int>(str.size()), 0);
    printf("[%s %d] send: %d\n", __func__, __LINE__, len);
    if (len == 0)
    {
        return 1;
    }
    else if (len == SOCKET_ERROR)
    {
        printf("[%s %d] send error: %d\n", __func__, __LINE__, WSAGetLastError());
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
        printf("[%s %d] recv error: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    printf("[%s %d] recv len: %d buf: %s\n", __func__, __LINE__, len, recvData);

    return 0;
}

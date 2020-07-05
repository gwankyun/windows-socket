#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <MSWSock.h>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <string>
#include <functional>
#include "ScopeGuard/ScopeGuard.hpp"

enum class OperationType
{
    unknown = 0,
    recv,
    send
};

struct Operation
{
    OVERLAPPED overlapped;

    Operation() : type(OperationType::unknown)
    {
        memset(&overlapped, 0, sizeof(OVERLAPPED));
        buffer.buf = NULL;
        buffer.len = 0;
    }

    ~Operation()
    {
        if (buffer.buf != NULL)
        {
            delete buffer.buf;
        }
    }

    WSABUF buffer;
    OperationType type;
};

struct CompletionKey
{
    SOCKET socket;
};

DWORD WINAPI process(LPVOID lpParam)
{
    HANDLE competionPort = (HANDLE)lpParam;
    DWORD bytesTransferred = 0;
    CompletionKey* completionKey = NULL;
    Operation* operation = NULL;

    while (true)
    {
        if (GetQueuedCompletionStatus(
            competionPort,
            &bytesTransferred,
            (PULONG_PTR)&completionKey,
            (LPOVERLAPPED*)&operation,
            INFINITE) == 0)
        {
            SOCKET socket = completionKey->socket;
            if (operation == NULL)
            {
                printf("[%s %d] GetQueuedCompletionStatus socket: %d error: %d\n", __func__, __LINE__,
                    static_cast<int>(socket), GetLastError());
                closesocket(socket);
                delete completionKey;
                return 0;
            }
            else
            {
                printf("[%s %d] GetQueuedCompletionStatus socket: %d error: %d\n", __func__, __LINE__,
                    static_cast<int>(completionKey->socket), GetLastError());
                closesocket(socket);
                delete completionKey;
                delete operation;
            }
            continue;
        }

        switch (operation->type)
        {
        case OperationType::recv:
        {
            SOCKET client = completionKey->socket;
            if (bytesTransferred == 0)
            {
                closesocket(client);
                delete completionKey;
                delete operation;
                continue;
            }
            printf("[%s %d] recv: %s\n", __func__, __LINE__, operation->buffer.buf);
            delete operation;

            Operation* sendOperation = new Operation();
            std::string str = "Data from server!";
            sendOperation->buffer.buf = new CHAR[str.size()];
            memcpy(sendOperation->buffer.buf, str.c_str(), str.size());
            sendOperation->buffer.len = static_cast<ULONG>(str.size());
            sendOperation->type = OperationType::send;

            DWORD flags = 0;
            DWORD bytesdRecvd = 0;
            int result = WSASend(client, &sendOperation->buffer, 1, &bytesdRecvd, flags, &sendOperation->overlapped, NULL);
            if (result != 0)
            {
                int error = WSAGetLastError();
                if (error != WSA_IO_PENDING)
                {
                    printf("[%s %d] WSASend: %d\n", __func__, __LINE__, error);
                }
            }
            break;
        }
        case OperationType::send:
        {
            SOCKET client = completionKey->socket;
            if (bytesTransferred == 0)
            {
                closesocket(client);
                delete completionKey;
                delete operation;
                continue;
            }
            printf("[%s %d] send: %d\n", __func__, __LINE__, bytesTransferred);
            delete operation;
            break;
        }
        default:
            break;
        }
    }
}

int main()
{
    int result;
    WORD wVersionRequested = MAKEWORD(2, 2);
    WSADATA lpWSAData;

    result = WSAStartup(wVersionRequested, &lpWSAData);

    if (result != 0)
    {
        printf("[%s %d] WSAStartup failed: %d\n", __func__, __LINE__, result);
        return 1;
    }

    ON_SCOPE_EXIT(WSACleanup);

    HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completionPort == NULL)
    {
        printf("[%s %d] CreateIoCompletionPort failed\n", __func__, __LINE__);
        return 1;
    }

    SYSTEM_INFO systemInfo;
    GetSystemInfo(&systemInfo);

    for (size_t i = 0; i < (size_t)systemInfo.dwNumberOfProcessors * 2; i++)
    {
        HANDLE handle = CreateThread(NULL, 0, process, completionPort, 0, NULL);
        if (handle)
        {
            CloseHandle(handle);
        }
    }

    SOCKET server = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    SOCKADDR_IN addrServer;
    addrServer.sin_family = AF_INET;
    addrServer.sin_addr.s_addr = htonl(INADDR_ANY);
    addrServer.sin_port = htons(12345);

    result = bind(server, reinterpret_cast<SOCKADDR*>(&addrServer), sizeof(SOCKADDR));
    if (result != 0)
    {
        printf("[%s %d] bind: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    result = listen(server, SOMAXCONN);
    if (result != 0)
    {
        printf("[%s %d] listen: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    LPFN_ACCEPTEX acceptEx;
    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    result=  WSAIoctl(
        server,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &acceptEx,
        sizeof(acceptEx),
        &bytes,
        NULL,
        NULL);
    if (result != 0)
    {
        printf("[%s %d] WSAIoctl: %d\n", __func__, __LINE__, WSAGetLastError());
        return 1;
    }

    while (true)
    {
        SOCKET client = INVALID_SOCKET;
        client = WSAAccept(server, NULL, NULL, NULL, 0);
        if (client == INVALID_SOCKET)
        {
            printf("[%s %d] WSAAccept: %d\n", __func__, __LINE__, WSAGetLastError());
            continue;
        }

        CompletionKey* completionKey = new CompletionKey();
        completionKey->socket = client;
        HANDLE handle = CreateIoCompletionPort((HANDLE)client, completionPort, (ULONG_PTR)completionKey, 0);
        if (handle == NULL)
        {
            printf("[%s %d] CreateIoCompletionPort failed\n", __func__, __LINE__);
            delete completionKey;
            completionKey = NULL;
        }

        Operation* operation = new Operation();
        operation->buffer.buf = new CHAR[1024];
        memset(operation->buffer.buf, 0, 1024);
        operation->buffer.len = 1024;
        operation->type = OperationType::recv;

        DWORD flags = 0;
        DWORD bytesdRecvd = 0;
        result = WSARecv(client, &operation->buffer, 1, &bytesdRecvd, &flags, &operation->overlapped, NULL);
        if (result != 0)
        {
            int error = WSAGetLastError();
            if (error != WSA_IO_PENDING)
            {
                printf("[%s %d] WSARecv failed: %d\n", __func__, __LINE__, error);
            }
        }
    }

    DWORD dwByteTrans;
    PostQueuedCompletionStatus(completionPort, dwByteTrans, NULL, NULL);

    return 0;
}

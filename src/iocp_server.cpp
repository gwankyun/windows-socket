#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <MSWSock.h>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <string>
#include <functional>
#include <cassert>
#include "ScopeGuard/ScopeGuard.hpp"
#include "log.hpp"

LPFN_ACCEPTEX acceptEx;

enum class OperationType : uint8_t
{
    unknown = 0,
    recv,
    send,
    accept
};

struct Operation
{
    OVERLAPPED overlapped;
    WSABUF buffer;
    SOCKET socket;
    OperationType type;

    Operation() : type(OperationType::unknown), socket(INVALID_SOCKET)
    {
        memset(&overlapped, 0, sizeof(OVERLAPPED));
        buffer.buf = NULL;
        buffer.len = 0;
    }

    ~Operation()
    {
        if (buffer.buf != NULL)
        {
            delete[] buffer.buf;
        }
        LOG();
    }

};

SOCKET g_client;

void async_accept(SOCKET server)
{
    Operation* operation = new Operation();
    operation->buffer.buf = new CHAR[1024];
    memset(operation->buffer.buf, 0, 1024);
    operation->buffer.len = 1024;
    operation->type = OperationType::accept;
    operation->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //g_client = operation->socket;
    LOG("%d", operation);
    LOG("client: %d", operation->socket);

    DWORD bytesReceived;
    BOOL result = acceptEx(
        server, operation->socket,
        &operation->buffer, 0,
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
        //&bytesReceived, &(operation->overlapped));
        &bytesReceived, (LPOVERLAPPED)operation);
    if (result == FALSE)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            LOG("AcceptEx error: %d", error);
        }
    }
}

void async_recv(SOCKET socket_, char* buf_, uint32_t len_)
{

}

struct CompletionKey
{
    SOCKET socket;
};

DWORD WINAPI process(LPVOID lpParam)
{
    HANDLE completionPort = (HANDLE)lpParam;
    DWORD bytesTransferred = 0;
    CompletionKey* completionKey = NULL;
    Operation* operation = NULL;
    //LPOVERLAPPED overlapped = NULL;

    while (true)
    {
        if (GetQueuedCompletionStatus(
            completionPort,
            &bytesTransferred,
            (PULONG_PTR)&completionKey,
            (LPOVERLAPPED*)&operation,
            //(LPOVERLAPPED*)&overlapped,
            INFINITE) == 0)
        {
            SOCKET socket = completionKey->socket;
            if (operation == NULL)
            //if (overlapped == NULL)
            {
                LOG("GetQueuedCompletionStatus socket: %d error: %d", static_cast<int>(socket), GetLastError());
                //closesocket(socket);
                //delete completionKey;
                return 0;
            }
            else
            {
                LOG("GetQueuedCompletionStatus socket: %d error: %d", static_cast<int>(socket), GetLastError());
                //closesocket(socket);
                //delete completionKey;
                //delete operation;
            }
            closesocket(socket);
            delete completionKey;
            delete operation;
            continue;
        }

        //Operation* operation = (Operation*)(overlapped);

        LOG("socket: %d", completionKey->socket);

        switch (operation->type)
        {
        case OperationType::accept:
        {
            //setsockopt(operation->socket, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT,
            //    (char*)&(completionKey->socket), sizeof(completionKey->socket));
            LOG("%d", operation);
            SOCKET server = completionKey->socket;
            SOCKET client = operation->socket;
            LOG("client: %d", client);
            //delete operation;
            //Operation* recvOperation = new Operation();
            Operation* recvOperation = operation;
            delete[] recvOperation->buffer.buf;
            recvOperation->buffer.buf = new CHAR[1024];
            memset(recvOperation->buffer.buf, 0, 1024);
            recvOperation->buffer.len = 1024;
            recvOperation->type = OperationType::recv;
            recvOperation->socket = client;

            CompletionKey* clientCompletionKey = new CompletionKey();
            clientCompletionKey->socket = client;
            LOG("completionPort: %d", completionPort);
            //assert(client == g_client);
            HANDLE handle = CreateIoCompletionPort((HANDLE)client, completionPort, (ULONG_PTR)clientCompletionKey, 0);
            if (handle == NULL)
            {
                LOG("error: %d", GetLastError());
            }

            DWORD flags = 0;
            DWORD bytesdRecvd = 0;
            int result = WSARecv(client, &recvOperation->buffer, 1, &bytesdRecvd, &flags, &recvOperation->overlapped, NULL);
            if (result != 0)
            {
                int error = WSAGetLastError();
                if (error != WSA_IO_PENDING)
                {
                    LOG("WSARecv error: %d", error);
                }
            }
            //delete operation;
            async_accept(server);
            break;
        }
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
            LOG("recv: %s", operation->buffer.buf);
            //delete operation;

            //Operation* sendOperation = new Operation();
            Operation* sendOperation = operation;
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
                    LOG("WSASend error: %d", error);
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
            LOG("send: %d", bytesTransferred);
            //delete operation;
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
        LOG("WSAStartup failed");
        return 1;
    }

    ON_SCOPE_EXIT(WSACleanup);

    HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (completionPort == NULL)
    {
        LOG("CreateIoCompletionPort failed");
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
        LOG("bind error: %d", WSAGetLastError());
        return 1;
    }

    result = listen(server, SOMAXCONN);
    if (result != 0)
    {
        LOG("listen error: %d", WSAGetLastError());
        return 1;
    }
    LOG("server: %d", server);

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
        LOG("WSAIoctl error: %d", WSAGetLastError());
        return 1;
    }

    CompletionKey* completionKey = new CompletionKey();
    completionKey->socket = server;
    HANDLE handle =  CreateIoCompletionPort((HANDLE)server, completionPort, (ULONG_PTR)completionKey, 0);
    if (handle == NULL)
    {
        LOG("error: %d", GetLastError());
    }
    LOG("completionPort: %d", completionPort);

    async_accept(server);

    while (true)
    {
        Sleep(100);
    }

    DWORD dwByteTrans = 0;
    PostQueuedCompletionStatus(completionPort, dwByteTrans, NULL, NULL);

    return 0;
}

#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <MSWSock.h>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <string>
#include <functional>
#include "ScopeGuard/ScopeGuard.hpp"

LPFN_ACCEPTEX acceptEx;

enum class OperationType
{
    accept,
    recv,
    send,
    connect
};

struct Context
{
    OVERLAPPED overlapped;
    WSABUF data;
    OperationType type;
    SOCKET socket;
    char* buf;
    uint32_t len;
    std::function<void(int32_t, uint32_t)> callback;
    std::function<void(int32_t, Context*, SOCKET)> accept;
};

struct CompletionKey
{
    SOCKET socket;
};

template<typename F>
void async_accept(SOCKET server, F callback)
{
    printf("[%s %d]\n", __func__, __LINE__);
    //CompletionKey* key = new CompletionKey();
    Context* context = new Context();

    context->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    context->type = OperationType::accept;
    context->data.buf = new char[128];
    context->data.len = 128;
    context->accept = callback;
    //std::cout << "async_accept: " << overlapped->socket << std::endl;

    printf("[%s %d] socket: %d\n", __func__, __LINE__, context->socket);

    DWORD bytesReceived;
    BOOL result = acceptEx(
        server, context->socket,
        &context->data, 0,
        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
        &bytesReceived, &(context->overlapped));
    if (result == FALSE)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            callback(error, NULL, INVALID_SOCKET);
        }
    }
}

template<typename F>
void async_recv(SOCKET socket, Context* context, char* buf, uint32_t len, F callback)
{
    printf("[%s %d] socket: %d\n", __func__, __LINE__, socket);
    DWORD flags = 0;
    DWORD bytesdRecvd = 0;
    context->callback = callback;
    context->type = OperationType::recv;
    context->data.buf = buf;
    context->data.len = len;
    memset(&context->overlapped, 0, sizeof(OVERLAPPED));
    int result = WSARecv(socket, &context->data, 1, &bytesdRecvd, &flags, &context->overlapped, NULL);
    if (result != 0)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            printf("[%s %d] err: %d\n", __func__, __LINE__, error);
            callback(error, 0);
        }
    }
}

template<typename F>
void async_send(SOCKET socket, Context* context, char* buf, uint32_t len, F callback)
{
    context->data.buf = buf;
    context->data.len = len;
    context->type = OperationType::send;
    context->callback = callback;
    DWORD flags = 0;
    DWORD bytesdRecvd = 0;
    context->type = OperationType::send;
    int result = WSASend(socket, &context->data, 1, &bytesdRecvd, flags, &context->overlapped, NULL);
    if (result != 0)
    {
        int error = WSAGetLastError();
        if (error != ERROR_IO_PENDING)
        {
            callback(error, 0);
        }
    }
}

void on_send(int32_t err, uint32_t size, SOCKET socket, Context* context, char* buf)
{
    if (err)
    {
        printf("%s: %d\n", __func__, err);
        return;
    }
    printf("send: %d\n", size);
    delete[] buf;
}

void on_recv(int32_t err, uint32_t size, SOCKET socket, Context* context, char* buf)
{
    if (err)
    {
        printf("[%s %d] err: %d\n", __func__, __LINE__, err);
        return;
    }
    std::cout << buf << std::endl;
    delete[] buf;
    std::string str = "Data from server!";
    char* buffer = new char[str.size()];
    memcpy(buffer, const_cast<const char*>(str.c_str()), str.size());
    async_send(socket, context, buffer, str.size(), [socket, context, buffer](int32_t err, uint32_t size) {
        on_send(err, size, socket, context, buffer);
    });
}

void on_accept(int32_t err, Context* context, SOCKET socket, SOCKET server)
{
    if (err)
    {
        printf("%s: %d\n", __func__, err);
        return;
    }
    async_accept(server, [err, server](int32_t e, Context* o, SOCKET s) {
        on_accept(e, o, s, server);
    });

    char* buf = new char[1024];
    memset(buf, 0, 1024);
    async_recv(socket, context, buf, 1024,
        [socket, context, buf](int32_t err, uint32_t size)
    {
        on_recv(err, size, socket, context, buf);
    });
}

DWORD WINAPI process(LPVOID lpParam)
{
    HANDLE completionPort = (HANDLE)lpParam;
    DWORD bytesTransferred = 0;
    CompletionKey* completionKey = NULL;
    Context* context = NULL;

    while (true)
    {
        if (GetQueuedCompletionStatus(
            completionPort,
            &bytesTransferred,
            (PULONG_PTR)&completionKey,
            (LPOVERLAPPED*)&context,
            INFINITE) == 0)
        {
            int err = GetLastError();
            if (err == WAIT_TIMEOUT || err == ERROR_NETNAME_DELETED)
            {
                std::cout << "closing socket: " << completionKey->socket << std::endl;
                //closesocket(completionKey->socket);
                //delete completionKey;
                //if (context->data.buf != NULL)
                //{
                //    delete[] context->data.buf;
                //    context->data.buf = NULL;
                //}
                //delete context;
                continue;
            }
            else
            {
                printf("GetQueuedCompletionStatus: %d\n", err);
            }
            return 0;
        }

        //printf("[%s] completionKey: %d\n", completionKey->socket);

        //std::cout << "completionKey: " << completionKey->socket << std::endl;
        //std::cout << "overlapped: " << content->socket << std::endl;

        switch (context->type)
        {
        case OperationType::accept:
        {
            //printf("[%s] completionKey: %d\n", completionKey->socket);
            //std::cout << "accept" << std::endl;
            //std::cout << "client: " << content->socket << std::endl;
            SOCKET server = completionKey->socket;
            SOCKET client = context->socket;

            //delete[] context->data.buf;
            //context->data.buf = NULL;

            CompletionKey* clientCompletionKey = new CompletionKey();
            clientCompletionKey->socket = client;
            CreateIoCompletionPort((HANDLE)context->socket, completionPort, (ULONG_PTR)clientCompletionKey, 0);
            Context* clientContext = new Context();
            memset(clientContext, 0, sizeof(Context));
            clientContext->data.buf = NULL;

            context->accept(0, clientContext, client);
        }
        break;
        case OperationType::recv:
        {
            context->data.buf = NULL;
            context->callback(0, bytesTransferred);
        }
            break;
        case OperationType::send:
        {
            context->data.buf = NULL;
            context->callback(0, bytesTransferred);
        }
            break;
        default:
            break;
        }

        //if (bytesTransferred == 0)
        //{
        //    std::cout << "closing socket: " << completionKey->socket << std::endl;
        //    closesocket(completionKey->socket);
        //    delete completionKey;
        //    delete[] overlapped->data.buf;
        //    delete overlapped;
        //    continue;
        //}
    }
}

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

    HANDLE completionPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (!completionPort)
    {
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
    std::cout << "server: " << server << std::endl;

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

    err = listen(server, SOMAXCONN);
    if (err != 0)
    {
        std::cout << "listen: " << WSAGetLastError() << std::endl;
        return 1;
    }
    std::cout << "listen" << std::endl;

    GUID guidAcceptEx = WSAID_ACCEPTEX;
    DWORD bytes = 0;
    WSAIoctl(
        server,
        SIO_GET_EXTENSION_FUNCTION_POINTER,
        &guidAcceptEx,
        sizeof(guidAcceptEx),
        &acceptEx,
        sizeof(acceptEx),
        &bytes,
        NULL,
        NULL);

    //while (true)
    //{
    //    //SOCKET client = INVALID_SOCKET;
    //    //CompletionKey* completionKey = NULL;
    //    //Overlapped* overlapped = NULL;
    //    //client = WSAAccept(server, NULL, NULL, NULL, 0);
    //    //std::cout << "WSAAccept" << std::endl;

    //    //completionKey = new CompletionKey();
    //    //completionKey->socket = client;
    //    //CreateIoCompletionPort((HANDLE)client, completionPort, (ULONG_PTR)completionKey, 0);
    //    //
    //    //overlapped = new Overlapped();
    //    //memset(overlapped, 0, sizeof(Overlapped));
    //    //overlapped->data.buf = new CHAR[1024];
    //    //memset(overlapped->data.buf, 0, 1024);
    //    //overlapped->data.len = 1024;

    //    Overlapped* overlapped = new Overlapped();
    //    overlapped->data.buf = new CHAR[1024];
    //    memset(overlapped->data.buf, 0, 1024);
    //    overlapped->data.len = 1024;

    //    overlapped->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);

    //    BOOL result = acceptEx(
    //        server, overlapped->socket,
    //        &overlapped->data, 0,
    //        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
    //        &bytes, &(overlapped->overlapped));

    //    DWORD flags = 0;
    //    DWORD bytesdRecvd = 0;
    //    WSARecv(client, &overlapped->data, 1, &bytesdRecvd, &flags, &overlapped->overlapped, NULL);
    //}


    CompletionKey* completionKey = new CompletionKey();
    completionKey->socket = server;
    CreateIoCompletionPort((HANDLE)server, completionPort, (ULONG_PTR)completionKey, 0);

    //{
    //    Overlapped* overlapped = new Overlapped();

    //    overlapped->socket = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    //    overlapped->type = OperationType::accept;

    //    BOOL result = acceptEx(
    //        server, overlapped->socket,
    //        &overlapped->data, 0,
    //        sizeof(sockaddr_in) + 16, sizeof(sockaddr_in) + 16,
    //        &bytes, &(overlapped->overlapped));
    //}
    async_accept(server, [server](int32_t e, Context* o, SOCKET s) {
        on_accept(e, o, s, server);
    });

    while (true)
    {
        Sleep(100);
    }

    DWORD dwByteTrans = 0;
    PostQueuedCompletionStatus(completionPort, dwByteTrans, NULL, NULL);

    return 0;
}

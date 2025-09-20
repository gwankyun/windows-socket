//#include "stdafx.h"
//#include <Afx.h>
//#include <Windows.h>
#include <Winsock2.h>
#pragma comment(lib, "WS2_32.lib")

#include <mswsock.h>    //微软扩展的类库
#include <cstdio>

#define DATA_BUFSIZE 100
#define READ   0
#define WRITE  1
#define ACCEPT 2

DWORD g_count = 0;
//扩展的输入输出结构
typedef struct _io_operation_data
{
    OVERLAPPED	overlapped;
    WSABUF		databuf;
    CHAR		buffer[DATA_BUFSIZE];
    BYTE		type;
    DWORD		len;
    SOCKET		sock;
}IO_OPERATION_DATA, * LP_IO_OPERATION_DATA;

//完成键
typedef struct _completion_key
{
    SOCKET sock;
    char   sIP[30];		//本机测试，IP都是127.0.0.1，没啥意思，实际写时候这个值填的是端口号
}COMPLETION_KEY, * LP_COMPLETION_KEY;

///////////////////////////////////////////////////
//完成端口句柄
HANDLE g_hComPort = NULL;
BOOL   g_bRun = FALSE;

BOOL AcceptClient(SOCKET sListen);		//发起接收连接操作
BOOL Recv(COMPLETION_KEY* pComKey, IO_OPERATION_DATA* pIO);	//发起接收操作

//处理重叠结果
BOOL ProcessIO(IO_OPERATION_DATA* pIOdata, COMPLETION_KEY* pComKey);

//////////////////////////////////////////////////
//服务线程
DWORD WINAPI ServerWorkerThread(LPVOID pParam);

//////////////////////////////////////////////////
LPFN_ACCEPTEX lpfnAcceptEx = NULL;					 //AcceptEx函数指针
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;  //加载GetAcceptExSockaddrs函数指针

///////////////////////////////////////////////////
//监听套接字,其实也不一定要是全局的。用于接收到连接后继续发起等待连接操作。
SOCKET g_sListen;

int main(int argc, char* argv[])
{
    g_bRun = TRUE;

    //创建完成端口
    g_hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_hComPort == NULL)
    {
        printf("Create completionport error!   %d\n", WSAGetLastError());
        return 0;
    }

    //创建服务线程
    SYSTEM_INFO sysInfor;
    GetSystemInfo(&sysInfor);
    int i = 0;
    for (i = 0; i < sysInfor.dwNumberOfProcessors * 2; i++)
        //	if(true)
    {
        HANDLE hThread;
        DWORD  dwThreadID;

        hThread = CreateThread(NULL, 0, ServerWorkerThread, g_hComPort, 0, &dwThreadID);
        CloseHandle(hThread);
    }


    //加载套接字库
    WSADATA wsData;
    if (0 != WSAStartup(0x0202, &wsData))
    {
        printf("加载套接字库失败!   %d\n", WSAGetLastError());
        g_bRun = FALSE;
        return 0;
    }

    ////////////////////////////////////////////////////////////////////
    //等待客户端连接

    //先创建一个套接字用于监听
    SOCKET sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    g_sListen = sListen;

    //将监听套接字与完成端口绑定
    LP_COMPLETION_KEY pComKey;		//完成键
    //pComKey = (LP_COMPLETION_KEY)GlobalAlloc(GPTR, sizeof(COMPLETION_KEY));
    pComKey = new COMPLETION_KEY();
    pComKey->sock = sListen;
    CreateIoCompletionPort((HANDLE)sListen, g_hComPort, (DWORD)pComKey, 0);

    //监听套接字绑定监听
    SOCKADDR_IN serAdd;
    serAdd.sin_addr.S_un.S_addr = htonl(INADDR_ANY);
    serAdd.sin_family = AF_INET;
    serAdd.sin_port = htons(12345);

    bind(sListen, (SOCKADDR*)&serAdd, sizeof(SOCKADDR));
    listen(sListen, 5);
    if (sListen == SOCKET_ERROR)
    {
        goto STOP_SERVER;
    }

    /////////////////////////////////////////////////////////////////////
    //使用WSAIoctl获取AcceptEx函数指针
    if (true)
    {
        DWORD dwbytes = 0;
        //Accept function GUID
        GUID guidAcceptEx = WSAID_ACCEPTEX;

        if (0 != WSAIoctl(sListen, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidAcceptEx, sizeof(guidAcceptEx),
            &lpfnAcceptEx, sizeof(lpfnAcceptEx),
            &dwbytes, NULL, NULL))
        {
            //百度百科,有关该函数的所有返回值都有！
        }


        // 获取GetAcceptExSockAddrs函数指针，也是同理
        GUID guidGetAcceptExSockaddrs = WSAID_GETACCEPTEXSOCKADDRS;
        if (0 != WSAIoctl(sListen, SIO_GET_EXTENSION_FUNCTION_POINTER,
            &guidGetAcceptExSockaddrs,
            sizeof(guidGetAcceptExSockaddrs),
            &lpfnGetAcceptExSockaddrs,
            sizeof(lpfnGetAcceptExSockaddrs),
            &dwbytes, NULL, NULL))
        {
        }
    }

    //发起接收的异步操作
    AcceptClient(sListen);

    //不让主线程退出
    while (g_bRun)
    {
        Sleep(1000);
    }

STOP_SERVER:
    closesocket(sListen);
    g_bRun = FALSE;
    WSACleanup();

    return 0;
}

/////////////////////////////////////////////////////////////////////////
//服务线程
DWORD WINAPI ServerWorkerThread(LPVOID pParam)
{
    HANDLE  completionPort = (HANDLE)pParam;
    DWORD	dwIoSize;

    COMPLETION_KEY* pComKey;		//完成键
    LP_IO_OPERATION_DATA  lpIOoperData;		//I/O数据

    //用于发起接收重叠操作
    BOOL bRet;

    while (g_bRun)
    {
        bRet = FALSE;

        dwIoSize = -1;
        pComKey = NULL;
        lpIOoperData = NULL;

        bRet = GetQueuedCompletionStatus(g_hComPort, &dwIoSize, (PULONG_PTR)&pComKey, (LPOVERLAPPED*)&lpIOoperData, INFINITE);

        if (!bRet)
        {
            DWORD dwIOError = GetLastError();
            if (WAIT_TIMEOUT == dwIOError)
            {
                continue;
            }
            else if (NULL != lpIOoperData)
            {
                CancelIo((HANDLE)pComKey->sock);	//取消等待执行的异步操作
                closesocket(pComKey->sock);
                //GlobalFree(pComKey);
                delete pComKey;
            }
            else
            {
                g_bRun = FALSE;
                break;
            }
        }
        else
        {
            if (0 == dwIoSize && (READ == lpIOoperData->type || WRITE == lpIOoperData->type))
            {
                printf("客户断开了连接!\n");
                CancelIo((HANDLE)pComKey->sock);	//取消等待执行的异步操作
                closesocket(pComKey->sock);
                //GlobalFree(pComKey);
                delete pComKey;
                //GlobalFree(lpIOoperData);
                delete lpIOoperData;
                continue;
            }
            else
            {
                ProcessIO(lpIOoperData, pComKey);
            }
        }
    }

    return 0;
}

BOOL ProcessIO(IO_OPERATION_DATA* pIOoperData, COMPLETION_KEY* pComKey)
{
    if (pIOoperData->type == READ)
    {
    }
    else if (pIOoperData->type == WRITE)
    {
    }
    else if (pIOoperData->type == ACCEPT)
    {	//使用GetAcceptExSockaddrs函数 获得具体的各个地址参数.
        printf("accept sucess!\n");
        printf("ProcessIO: %d\n", pIOoperData->sock);
        setsockopt(pIOoperData->sock, SOL_SOCKET, SO_UPDATE_ACCEPT_CONTEXT, (char*)&(pComKey->sock), sizeof(pComKey->sock));

        //LP_COMPLETION_KEY pClientComKey = (LP_COMPLETION_KEY)GlobalAlloc(GPTR, sizeof(COMPLETION_KEY));
        LP_COMPLETION_KEY pClientComKey = new COMPLETION_KEY();
        pClientComKey->sock = pIOoperData->sock;

        SOCKADDR_IN* addrClient = NULL, * addrLocal = NULL;
        int nClientLen = sizeof(SOCKADDR_IN), nLocalLen = sizeof(SOCKADDR_IN);

        lpfnGetAcceptExSockaddrs(pIOoperData->buffer, 0,
            sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
            (LPSOCKADDR*)&addrLocal, &nLocalLen,
            (LPSOCKADDR*)&addrClient, &nClientLen);

        sprintf(pClientComKey->sIP, "%d\n", addrClient->sin_port);	//cliAdd.sin_port );
        printf(pClientComKey->sIP);
        return TRUE;

        CreateIoCompletionPort((HANDLE)pClientComKey->sock, g_hComPort, (DWORD)pClientComKey, 0);	//将监听到的套接字关联到完成端口



        //	char s[30] = {0};
        //	sprintf( s, "%d\n", g_count++ );
        //	printf(s);
            //接收到一个连接，就再发起一个异步操作！
        AcceptClient(g_sListen);
    }

    return TRUE;
}


BOOL AcceptClient(SOCKET sListen)
{
    DWORD dwBytes;

    LP_IO_OPERATION_DATA pIO;
    //pIO = (LP_IO_OPERATION_DATA)GlobalAlloc(GPTR, sizeof(IO_OPERATION_DATA));
    pIO = new IO_OPERATION_DATA();
    pIO->databuf.buf = pIO->buffer;
    pIO->databuf.len = pIO->len = DATA_BUFSIZE;
    pIO->type = ACCEPT;

    //先创建一个套接字(相比accept有点就在此,accept是接收到连接才创建出来套接字,浪费时间. 这里先准备一个,用于接收连接)
    pIO->sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    printf("AcceptClient: %d\n", pIO->sock);

    //调用AcceptEx函数，地址长度需要在原有的上面加上16个字节
    //向服务线程投递一个接收连接的的请求
    BOOL rc = lpfnAcceptEx(sListen, pIO->sock,
        pIO->buffer, 0,
        sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16,
        &dwBytes, &(pIO->overlapped));

    if (FALSE == rc)
    {
        if (WSAGetLastError() != ERROR_IO_PENDING)
        {
            printf("%d", WSAGetLastError());
            return false;
        }
    }

    return true;
}

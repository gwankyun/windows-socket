//#include "stdafx.h"
//#include <Afx.h>
//#include <Windows.h>
#include <Winsock2.h>
#pragma comment(lib, "WS2_32.lib")

#include <mswsock.h>    //΢����չ�����
#include <cstdio>

#define DATA_BUFSIZE 100
#define READ   0
#define WRITE  1
#define ACCEPT 2

DWORD g_count = 0;
//��չ����������ṹ
typedef struct _io_operation_data
{
    OVERLAPPED	overlapped;
    WSABUF		databuf;
    CHAR		buffer[DATA_BUFSIZE];
    BYTE		type;
    DWORD		len;
    SOCKET		sock;
}IO_OPERATION_DATA, * LP_IO_OPERATION_DATA;

//��ɼ�
typedef struct _completion_key
{
    SOCKET sock;
    char   sIP[30];		//�������ԣ�IP����127.0.0.1��ûɶ��˼��ʵ��дʱ�����ֵ����Ƕ˿ں�
}COMPLETION_KEY, * LP_COMPLETION_KEY;

///////////////////////////////////////////////////
//��ɶ˿ھ��
HANDLE g_hComPort = NULL;
BOOL   g_bRun = FALSE;

BOOL AcceptClient(SOCKET sListen);		//����������Ӳ���
BOOL Recv(COMPLETION_KEY* pComKey, IO_OPERATION_DATA* pIO);	//������ղ���

//�����ص����
BOOL ProcessIO(IO_OPERATION_DATA* pIOdata, COMPLETION_KEY* pComKey);

//////////////////////////////////////////////////
//�����߳�
DWORD WINAPI ServerWorkerThread(LPVOID pParam);

//////////////////////////////////////////////////
LPFN_ACCEPTEX lpfnAcceptEx = NULL;					 //AcceptEx����ָ��
LPFN_GETACCEPTEXSOCKADDRS lpfnGetAcceptExSockaddrs;  //����GetAcceptExSockaddrs����ָ��

///////////////////////////////////////////////////
//�����׽���,��ʵҲ��һ��Ҫ��ȫ�ֵġ����ڽ��յ����Ӻ��������ȴ����Ӳ�����
SOCKET g_sListen;

int main(int argc, char* argv[])
{
    g_bRun = TRUE;

    //������ɶ˿�
    g_hComPort = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);
    if (g_hComPort == NULL)
    {
        printf("Create completionport error!   %d\n", WSAGetLastError());
        return 0;
    }

    //���������߳�
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


    //�����׽��ֿ�
    WSADATA wsData;
    if (0 != WSAStartup(0x0202, &wsData))
    {
        printf("�����׽��ֿ�ʧ��!   %d\n", WSAGetLastError());
        g_bRun = FALSE;
        return 0;
    }

    ////////////////////////////////////////////////////////////////////
    //�ȴ��ͻ�������

    //�ȴ���һ���׽������ڼ���
    SOCKET sListen = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    g_sListen = sListen;

    //�������׽�������ɶ˿ڰ�
    LP_COMPLETION_KEY pComKey;		//��ɼ�
    //pComKey = (LP_COMPLETION_KEY)GlobalAlloc(GPTR, sizeof(COMPLETION_KEY));
    pComKey = new COMPLETION_KEY();
    pComKey->sock = sListen;
    CreateIoCompletionPort((HANDLE)sListen, g_hComPort, (DWORD)pComKey, 0);

    //�����׽��ְ󶨼���
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
    //ʹ��WSAIoctl��ȡAcceptEx����ָ��
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
            //�ٶȰٿ�,�йظú��������з���ֵ���У�
        }


        // ��ȡGetAcceptExSockAddrs����ָ�룬Ҳ��ͬ��
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

    //������յ��첽����
    AcceptClient(sListen);

    //�������߳��˳�
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
//�����߳�
DWORD WINAPI ServerWorkerThread(LPVOID pParam)
{
    HANDLE  completionPort = (HANDLE)pParam;
    DWORD	dwIoSize;

    COMPLETION_KEY* pComKey;		//��ɼ�
    LP_IO_OPERATION_DATA  lpIOoperData;		//I/O����

    //���ڷ�������ص�����
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
                CancelIo((HANDLE)pComKey->sock);	//ȡ���ȴ�ִ�е��첽����
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
                printf("�ͻ��Ͽ�������!\n");
                CancelIo((HANDLE)pComKey->sock);	//ȡ���ȴ�ִ�е��첽����
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
    {	//ʹ��GetAcceptExSockaddrs���� ��þ���ĸ�����ַ����.
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

        CreateIoCompletionPort((HANDLE)pClientComKey->sock, g_hComPort, (DWORD)pClientComKey, 0);	//�����������׽��ֹ�������ɶ˿�



        //	char s[30] = {0};
        //	sprintf( s, "%d\n", g_count++ );
        //	printf(s);
            //���յ�һ�����ӣ����ٷ���һ���첽������
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

    //�ȴ���һ���׽���(���accept�е���ڴ�,accept�ǽ��յ����ӲŴ��������׽���,�˷�ʱ��. ������׼��һ��,���ڽ�������)
    pIO->sock = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
    printf("AcceptClient: %d\n", pIO->sock);

    //����AcceptEx��������ַ������Ҫ��ԭ�е��������16���ֽ�
    //������߳�Ͷ��һ���������ӵĵ�����
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

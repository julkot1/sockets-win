#include "socket.h"
#include "message.h"


struct addrinfo *createServer(HWND hwnd)
{
    struct addrinfo *result = NULL, hints;
    WSADATA wsaData;
    int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (iResult != 0)
    {
        LogMessage(hwnd,"WSAStartup failed: %d", iResult);
        return NULL;
    }

    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags = AI_PASSIVE;

    iResult = getaddrinfo(NULL, DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        LogMessage(hwnd,"getaddrinfo failed: %d", iResult);
        WSACleanup();
        return NULL;
    }

    return result;
}



SOCKET createListenSocket(struct addrinfo *result, HWND hwnd)
{
    int iResult;
    printf("%d\n", action);
    SOCKET ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (ListenSocket == INVALID_SOCKET)
    {
        LogMessage(hwnd, "Error at socket(): %d", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return INVALID_SOCKET;
    }

    iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        LogMessage(hwnd,"bind failed with error: %d", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    freeaddrinfo(result);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        LogMessage(hwnd,"Listen failed with error: %ld", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return INVALID_SOCKET;
    }

    return ListenSocket;
}

int sendRequest(SOCKET ClientSocket, HWND hwnd)
{
    MESSAGE msg;
    if(action == GET_CLIPBOARD_ACTION)
    {
        action = NON_ACTION;
        msg.header = SIGNAL_CLIPBOARD;
        msg.hasNext = NEXT_FALSE;
        int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
        if (iSendResult == SOCKET_ERROR)
        {
            LogMessage(hwnd, "[Thread %lu] send failed: %d", GetCurrentThreadId(), WSAGetLastError());
        } else
            LogMessage(hwnd, "[Thread %lu] send SIGNAL: %d", GetCurrentThreadId(), msg.header);
        do
        {
            recv(ClientSocket, (char*)&msg, sizeof(MESSAGE ), 0);
            if (msg.header == GET_CLIPBOARD)
            {
                LogMessage(hwnd,msg.payload);
            }

        }while(msg.hasNext == NEXT_TRUE);

    }
    return 0;
}
DWORD WINAPI clientHandler(LPVOID clientSocketPtr)
{
    ThArguments args = *((ThArguments*)clientSocketPtr);
    SOCKET ClientSocket = *args.ClientSocket;
    HWND hwnd = *args.hwnd;

    MESSAGE msg;
    int iResult;
    int iSendResult;

    do
    {
        if(sendRequest(ClientSocket, hwnd))break;
//        iResult = recv(ClientSocket, (char*)&msg, sizeof (MESSAGE), 0);
//        if (iResult > 0)
//        {
//            LogMessage(hwnd,"[Thread %lu] Received: HEADER:%d PAYLOAD:%s", GetCurrentThreadId(), msg.header, msg.payload);
////            iSendResult = send(ClientSocket, recvbuf, iResult, 0);
////            if (iSendResult == SOCKET_ERROR)
////            {
////                LogMessage(hwnd, "[Thread %lu] send failed: %d", GetCurrentThreadId(), WSAGetLastError());
////                break;
////            }
//        }
//        else if (iResult == 0)
//        {
//            LogMessage(hwnd, "[Thread %lu] Connection closing...", GetCurrentThreadId());
//            *args.sockets--;
//            break;
//        } else
//        {
//            LogMessage(hwnd, "[Thread %lu] recv failed: %d", GetCurrentThreadId(), WSAGetLastError());
//            *args.sockets--;
//            break;
//        }
    } while (1);
    printf("%d\n", *args.sockets);

    closesocket(ClientSocket);
    return 0;
}


DWORD WINAPI serverThread(LPVOID lpParam)
{
    HWND hEditLog = *(HWND *)lpParam;
    WSADATA wsaData;
    struct addrinfo *result = NULL, hints;
    result = createServer(hEditLog);
    int sockets = 0;
    if (result == NULL)
    {
        printf("ERROR\n");
        return 1;
    }
    SOCKET ListenSocket = createListenSocket(result, hEditLog);

    if (listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        LogMessage(hEditLog, "listen failed: %ld", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    LogMessage(hEditLog,"Server started. Listening on port %s...", DEFAULT_PORT);

    while (1)
    {
        SOCKET* ClientSocket = (SOCKET*)malloc(sizeof(SOCKET));
        *ClientSocket = accept(ListenSocket, NULL, NULL);
        if (*ClientSocket == INVALID_SOCKET)
        {
            LogMessage(hEditLog,"accept failed: %d", WSAGetLastError());
            free(ClientSocket);
            continue;
        }

        LogMessage(hEditLog,"Client connected. Socket: %d", *ClientSocket);
        ThArguments arguments;
        arguments.hwnd = &hEditLog;
        arguments.ClientSocket = ClientSocket;
        arguments.sockets = &sockets;
        HANDLE hThread = CreateThread(NULL, 0, clientHandler, &arguments, 0, NULL);
        if (hThread)
        {
            sockets++;
            printf("%d\n", sockets);
            CloseHandle(hThread);
        } else
        {
            LogMessage(hEditLog,"Failed to create client thread.");
            closesocket(*ClientSocket);
            free(ClientSocket);
        }
    }

    closesocket(ListenSocket);
    WSACleanup();
    return 0;
}
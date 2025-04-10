#include "socket.h"
#include "message.h"
#include "actions.h"


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


DWORD WINAPI clientHandler(LPVOID clientSocketPtr)
{
    ThArguments args = *((ThArguments*)clientSocketPtr);
    SOCKET ClientSocket = *args.ClientSocket;
    HWND hwnd = *args.hwnd;
    int id = args.id;
    char name[16];
    sprintf(name, "ID: %d", id);
    SendMessage(menuController.clientSelect, CB_ADDSTRING, 2, (LPARAM)name);

    MESSAGE msg;
    int iResult;
    int iSendResult;

    do
    {
        if(sendRequest(ClientSocket, hwnd, id))break;
    } while (1);
    printf("%s\n", name);
    int index = SendMessage(menuController.clientSelect, CB_FINDSTRINGEXACT, -1, (LPARAM)name);
    SendMessage(menuController.clientSelect, CB_DELETESTRING, index, 0);
    menuController.selected = -1;

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
    int id = 0;
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
        arguments.id = id++;
        HANDLE hThread = CreateThread(NULL, 0, clientHandler, &arguments, 0, NULL);
        if (hThread)
        {
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
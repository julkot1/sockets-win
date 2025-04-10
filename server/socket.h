#ifndef SERVER_SOCKET_H
#define SERVER_SOCKET_H
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include "gui.h"
#define DEFAULT_BUFLEN 512

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "2137"

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

struct addrinfo *createServer(HWND hwnd);
SOCKET createListenSocket(struct addrinfo *result, HWND hwnd);
DWORD WINAPI clientHandler(LPVOID clientSocketPtr);
DWORD WINAPI serverThread(LPVOID lpParam);

typedef struct {
    HWND *hwnd;
    SOCKET *ClientSocket;
    int *sockets;
    int id;
} ThArguments;
#endif //SERVER_SOCKET_H

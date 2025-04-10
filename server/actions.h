#ifndef SERVER_ACTIONS_H
#define SERVER_ACTIONS_H
#include "socket.h"
#include "message.h"

int sendRequest(SOCKET ClientSocket, HWND hwnd, int id);
int getClipboard(SOCKET ClientSocket, HWND hwnd, int id);
int setClipboard(SOCKET ClientSocket, HWND hwnd, int id);
int closeClient(SOCKET socket, HWND pHwnd, int id);


#endif //SERVER_ACTIONS_H

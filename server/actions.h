#ifndef SERVER_ACTIONS_H
#define SERVER_ACTIONS_H
#include "socket.h"
#include "message.h"

int sendRequest(ThArguments *args);
int getClipboard(SOCKET ClientSocket, HWND hwnd, int id);
int setClipboard(SOCKET ClientSocket, HWND hwnd, int id);
int closeClient(SOCKET socket, HWND pHwnd, int id);
int mouseLock(SOCKET socket, HWND hwnd, int id, MOUSE_STATE *mouseState);



#endif //SERVER_ACTIONS_H

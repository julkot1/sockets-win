#ifndef SERVER_GUI_H
#define SERVER_GUI_H
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

void LogMessage(HWND  hEditLog, const char* format, ...);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void createWindow(HWND *hEditLog, HINSTANCE hInstance, int nCmdShow);

#endif //SERVER_GUI_H
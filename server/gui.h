#ifndef SERVER_GUI_H
#define SERVER_GUI_H
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "message.h"

void LogMessage(HWND  hEditLog, const char* format, ...);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void createWindow(HWND *hEditLog, HINSTANCE hInstance, int nCmdShow);
typedef enum {
    GET_CLIPBOARD_ACTION,
    CLOSE_ACTION,
    SET_CLIPBOARD_ACTION,
    NON_ACTION,
} ACTION;

typedef struct {
    ACTION action;
    HWND clientSelect;
    int selected;
    PAYLOAD payload;
} MENU_CONTROLLER;
extern MENU_CONTROLLER menuController;

#endif //SERVER_GUI_H
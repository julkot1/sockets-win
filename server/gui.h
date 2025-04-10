#ifndef SERVER_GUI_H
#define SERVER_GUI_H
#include <winsock2.h>
#include <windows.h>
#include <ws2tcpip.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include "message.h"


#define SET_CLIPBOARD_ID 3
#define CLOSE_ID 4
#define GET_CLIPBOARD_ID 1
#define GET_SCREENSHOT_ID 5
#define LOCK_MOUSE_ID 6
#define EXPLORE_FILES_ID 7
#define DOWNLOAD_FILE_ID 8
#define GET_PROGRAMS_ID 9
#define CLOSE_PROGRAM_ID 10


void LogMessage(HWND  hEditLog, const char* format, ...);
LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void createWindow(HWND *hEditLog, HINSTANCE hInstance, int nCmdShow);
typedef enum {
    GET_CLIPBOARD_ACTION,
    CLOSE_ACTION,
    SET_CLIPBOARD_ACTION,
    LOCK_MOUSE_ACTION,
    NON_ACTION,
} ACTION;

typedef struct {
    ACTION action;
    HWND clientSelect;
    HWND lockButton;
    int selected;
    PAYLOAD payload;
} MENU_CONTROLLER;
extern MENU_CONTROLLER menuController;

#endif //SERVER_GUI_H
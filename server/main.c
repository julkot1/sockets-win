#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include "socket.h"
#include "gui.h"
HWND hEditLog;

MENU_CONTROLLER menuController;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nCmdShow)
{
    menuController.action = NON_ACTION;
    menuController.selected = -1;
    createWindow(&hEditLog, hInstance, nCmdShow);
    CreateThread(NULL, 0, serverThread, &hEditLog, 0, NULL);

    MSG msg = {0};
    while (GetMessage(&msg, NULL, 0, 0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

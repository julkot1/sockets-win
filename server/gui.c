#include "gui.h"

void LogMessage(HWND hEditLog, const char* format, ...) {
    char buffer[512];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    int len = GetWindowTextLength(hEditLog);
    SendMessage(hEditLog, EM_SETSEL, (WPARAM)len, (LPARAM)len);
    SendMessage(hEditLog, EM_REPLACESEL, 0, (LPARAM)buffer);
    SendMessage(hEditLog, EM_REPLACESEL, 0, (LPARAM)"\r\n");
}


LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


void createWindow(HWND *hEditLog, HINSTANCE hInstance, int nCmdShow)
{
    const char CLASS_NAME[] = "TCPServerWindow";

    WNDCLASS wc = {0};
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = CLASS_NAME;

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
            0, CLASS_NAME, "TCP Server",
            WS_OVERLAPPEDWINDOW,
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
            NULL, NULL, hInstance, NULL);

    *hEditLog = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 10, 600, 400,
            hwnd, NULL, hInstance, NULL);

    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

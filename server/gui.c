#include "gui.h"

HWND hedit;
void LogMessage(HWND hEditLog, const char* format, ...)
{
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
char userInput[256] = {0}; // This will store the input text



LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_COMMAND:
            if (LOWORD(wParam) == 1)
                menuController.action = GET_CLIPBOARD_ACTION;
            else if (LOWORD(wParam) == 4)
                menuController.action = CLOSE_ACTION;
            else if ((LOWORD(wParam)) == 3)
            {
                GetWindowText(hedit, menuController.payload, 512);
                menuController.action = SET_CLIPBOARD_ACTION;
            }

            if (HIWORD(wParam) == CBN_SELCHANGE)
            {
                int selectedIndex = SendMessage(menuController.clientSelect, CB_GETCURSEL, 0, 0);
                char selectedText[256];
                SendMessage(menuController.clientSelect, CB_GETLBTEXT, selectedIndex, (LPARAM)selectedText);

                int idx = atoi(selectedText+4);
                menuController.selected = idx;
            }
           break;
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
            CW_USEDEFAULT, CW_USEDEFAULT, 640, 500,
            NULL, NULL, hInstance, NULL);

    *hEditLog = CreateWindowEx(
            WS_EX_CLIENTEDGE,
            "EDIT", "",
            WS_CHILD | WS_VISIBLE | WS_VSCROLL | ES_MULTILINE | ES_AUTOVSCROLL | ES_READONLY,
            10, 50, 600, 200,
            hwnd, NULL, hInstance, NULL);
    CreateWindow("BUTTON", "Get Clipboard",
                 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 260, 100, 30,
                 hwnd, (HMENU)1,
                 (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    CreateWindow("BUTTON", "Set Clipboard",
                 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 120, 260, 100, 30,
                 hwnd, (HMENU)3,
                 (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    CreateWindow("BUTTON", "Close",
                 WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,
                 10, 420, 100, 30,
                 hwnd, (HMENU)4,
                 (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL);
    hedit = CreateWindowEx(0, "EDIT", "",
                           WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
                           240, 262, 200, 25, hwnd, (HMENU)100, GetModuleHandle(NULL), NULL);
    HWND hLabel = CreateWindow(
            "STATIC",
            "Select client:",
            WS_CHILD | WS_VISIBLE,
            10, 10, 100, 25,
            hwnd,
            NULL,
            (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE),
            NULL
    );
    menuController.clientSelect = CreateWindow(
            "COMBOBOX", NULL,
            CBS_DROPDOWNLIST | WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_TABSTOP,
            110, 10, 150, 100,
            hwnd, (HMENU)2, (HINSTANCE)GetWindowLongPtr(hwnd, GWLP_HINSTANCE), NULL
    );
    ShowWindow(hwnd, nCmdShow);
    UpdateWindow(hwnd);
}

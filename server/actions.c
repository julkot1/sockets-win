#include "actions.h"

int sendRequest(ThArguments *args)
{
    if (args->id != menuController.selected)return 0;
    if(menuController.action == GET_CLIPBOARD_ACTION )
        return getClipboard(*args->ClientSocket, *args->hwnd, args->id);
    else if(menuController.action == CLOSE_ACTION)
        return closeClient(*args->ClientSocket, *args->hwnd, args->id);
    else if (menuController.action == SET_CLIPBOARD_ACTION)
        return setClipboard(*args->ClientSocket, *args->hwnd, args->id);
    else if (menuController.action == LOCK_MOUSE_ACTION)
        return mouseLock(*args->ClientSocket, *args->hwnd, args->id, &args->mouseState);
    else if (menuController.action == GET_FILES_ACTION)
        return getFiles(*args->ClientSocket, *args->hwnd, args->id);

    return 0;
}

int getFiles(SOCKET ClientSocket, HWND hwnd, int id)
{
    MESSAGE msg;
    menuController.action = NON_ACTION;
    msg.header = SIGNAL_FILES;
    msg.hasNext = NEXT_FALSE;

    strcpy(msg.payload, menuController.payload);
    printf("%s\n", msg.payload);
    int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        LogMessage(hwnd, "[Thread %lu] [Id %d] send failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    } else
        LogMessage(hwnd, "[Thread %lu] [Id %d] send SIGNAL: %d", GetCurrentThreadId(), id, msg.header);
    do {
        memset(msg.payload, 0, 512);

        int res = recv(ClientSocket, (char*)&msg, sizeof(MESSAGE ), 0);
        if(res == SOCKET_ERROR){
            LogMessage(hwnd, "[Thread %lu] [Id %d] received failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
            return 1;

        }
        if (msg.header == GET_FILES)
        {
            LogMessage(hwnd, "%s\n", msg.payload);
        }
    }while(msg.hasNext == NEXT_TRUE);
    LogMessage(hwnd, "[Thread %lu] [Id %d] End of receiving files.\n", GetCurrentThreadId(), id);



    return 0;
}
int mouseLock(SOCKET ClientSocket, HWND hwnd, int id, MOUSE_STATE *mouseState)
{
    MESSAGE msg;
    menuController.action = NON_ACTION;
    msg.header = *mouseState == MOUSE_UNLOCK ? SIGNAL_MOUSE_LOCK : SIGNAL_MOUSE_UNLOCK;
    if(*mouseState == MOUSE_UNLOCK )
    {
        *mouseState = MOUSE_LOCK;
    } else
    {
        *mouseState = MOUSE_UNLOCK;
    }
    msg.hasNext = NEXT_FALSE;

    strcpy(msg.payload, menuController.payload);

    int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        LogMessage(hwnd, "[Thread %lu] [Id %d] send failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    } else
        LogMessage(hwnd, "[Thread %lu] [Id %d] send SIGNAL: %d", GetCurrentThreadId(), id, msg.header);

    int res = recv(ClientSocket, (char*)&msg, sizeof(MESSAGE ), 0);
    if(res == SOCKET_ERROR){
        LogMessage(hwnd, "[Thread %lu] [Id %d] received failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    }
    LogMessage(hwnd, "[Thread %lu] [Id %d] received SIGNAL %d", GetCurrentThreadId(), id, msg.header);
    return 0;
}
int setClipboard(SOCKET ClientSocket, HWND hwnd, int id)
{
    MESSAGE msg;
    menuController.action = NON_ACTION;
    msg.header = SIGNAL_CLIPBOARD_SET;
    msg.hasNext = NEXT_FALSE;

    strcpy(msg.payload, menuController.payload);

    int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        LogMessage(hwnd, "[Thread %lu] [Id %d] send failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    } else
        LogMessage(hwnd, "[Thread %lu] [Id %d] send SIGNAL: %d", GetCurrentThreadId(), id, msg.header);

    int res = recv(ClientSocket, (char*)&msg, sizeof(MESSAGE ), 0);
    if(res == SOCKET_ERROR){
        LogMessage(hwnd, "[Thread %lu] [Id %d] received failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    }
    LogMessage(hwnd, "[Thread %lu] [Id %d] received SIGNAL %d", GetCurrentThreadId(), id, msg.header);
    return 0;
}

int closeClient(SOCKET ClientSocket, HWND hwnd, int id)
{
    MESSAGE msg;
    menuController.action = NON_ACTION;
    msg.header = SIGNAL_CLOSE;
    msg.hasNext = NEXT_FALSE;
    int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        LogMessage(hwnd, "[Thread %lu] [Id %d] send failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 0;
    } else
        LogMessage(hwnd, "[Thread %lu] [Id %d] send SIGNAL: %d", GetCurrentThreadId(), id, msg.header);
    return 1;

}

int getClipboard(SOCKET ClientSocket, HWND hwnd, int id)
{
    MESSAGE msg;
    menuController.action = NON_ACTION;
    msg.header = SIGNAL_CLIPBOARD;
    msg.hasNext = NEXT_FALSE;
    int iSendResult = send(ClientSocket, (char *) &msg, sizeof(MESSAGE), 0);
    if (iSendResult == SOCKET_ERROR)
    {
        LogMessage(hwnd, "[Thread %lu] [Id %d] send failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
        return 1;
    } else
        LogMessage(hwnd, "[Thread %lu] [Id %d] send SIGNAL: %d", GetCurrentThreadId(), id, msg.header);

    FILE *f = fopen("clipboard.txt", "w");
    do
    {
        memset(msg.payload, 0, 512);
        int res = recv(ClientSocket, (char*)&msg, sizeof(MESSAGE ), 0);
        if(res == SOCKET_ERROR){
            LogMessage(hwnd, "[Thread %lu] [Id %d] received failed: %d", GetCurrentThreadId(), id, WSAGetLastError());
            fclose(f);
            return 1;

        }
        if (msg.header == GET_CLIPBOARD)
        {
            int size_needed = MultiByteToWideChar(CP_UTF8, 0, msg.payload, -1, NULL, 0);
            LPWSTR lpwstr = (LPWSTR)malloc(size_needed * sizeof(WCHAR));
            if (lpwstr == NULL) {
                LogMessage(hwnd, "[Thread %lu] [Id %d] Memory allocation failed.\n", GetCurrentThreadId(), id);
                fclose(f);
                return 1;
            }

            MultiByteToWideChar(CP_UTF8, 0, msg.payload, -1, lpwstr, size_needed);

            if (lpwstr[size_needed-2] == 1){
                lpwstr[size_needed-2] = 0;
            }
            free(lpwstr);
        }

    }while(msg.hasNext == NEXT_TRUE);
    LogMessage(hwnd, "[Thread %lu] [Id %d] End of receiving clipboard.\n", GetCurrentThreadId(), id);
    fclose(f);
    return 0;
}
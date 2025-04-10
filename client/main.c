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

int main()
{
    LPCWSTR str = GetClipboardText();
    wprintf(L"Clipboard contents: %ls\n", str);
   // size_t sizeInBytes = (wcslen(str) + 1) * sizeof(wchar_t);
    //printf("l: %ld\n", sizeInBytes);



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
    hints.ai_protocol = IPPROTO_TCP;

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
        {
            sendClipboard(ConnectSocket);
            //msg.header = GET_CLIPBOARD;
            //char p[] = "foo";
            //strcpy(msg.payload, p);
            //send(ConnectSocket, (char *)&msg, sizeof(MESSAGE), 0);
        }

    }


    return 0;
}

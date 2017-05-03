#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 4096

DWORD WINAPI ClientThread(LPVOID);

int main() {
    BOOL   fConnected = FALSE;
    DWORD  dwThreadId = 0;
    HANDLE hPipe = INVALID_HANDLE_VALUE, hThread = NULL;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

    while(1) {
        printf("\nPipe Server: Main thread awaiting client connection on %ls\n", lpszPipename);
        hPipe = CreateNamedPipe(
            lpszPipename,
            PIPE_ACCESS_DUPLEX,
            PIPE_TYPE_MESSAGE |
            PIPE_READMODE_MESSAGE | 
            PIPE_WAIT,  
            PIPE_UNLIMITED_INSTANCES,
            BUFSIZE, 
            BUFSIZE, 
            0,  
            NULL);

        if (hPipe == INVALID_HANDLE_VALUE) {
            printf("CreateNamedPipe failed, GetLastError=%d.\n", GetLastError());
            return -1;
        }

        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : GetLastError() == ERROR_PIPE_CONNECTED;

        if (fConnected) {
            puts("Client connected, creating a processing thread.");

            hThread = CreateThread(
                NULL,
                0,
                ClientThread,
                (LPVOID)hPipe,
                0, 
                &dwThreadId);

            if (hThread == NULL) {
                printf("CreateThread failed, GetLastError=%d.\n", GetLastError());
                return -1;
            }
            CloseHandle(hThread);
        }
        else
            CloseHandle(hPipe);
    }

    return 0;
}

DWORD WINAPI ClientThread(LPVOID lpvParam) {
    HANDLE hHeap = GetProcessHeap();
    TCHAR* pchRequest = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));
    TCHAR* pchReply = (TCHAR*)HeapAlloc(hHeap, 0, BUFSIZE*sizeof(TCHAR));

    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;
    HANDLE hPipe = NULL;

    if (lpvParam == NULL) {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   ClientThread got an unexpected NULL value in lpvParam.\n");
        printf("   ClientThread exitting.\n");
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    if (pchRequest == NULL)  {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   ClientThread got an unexpected NULL heap allocation.\n");
        printf("   ClientThread exitting.\n");
        if (pchReply != NULL) HeapFree(hHeap, 0, pchReply);
        return (DWORD)-1;
    }

    if (pchReply == NULL) {
        printf("\nERROR - Pipe Server Failure:\n");
        printf("   ClientThread got an unexpected NULL heap allocation.\n");
        printf("   ClientThread exitting.\n");
        if (pchRequest != NULL) HeapFree(hHeap, 0, pchRequest);
        return (DWORD)-1;
    }

    printf("ClientThread created, receiving and processing messages.\n");

    hPipe = (HANDLE)lpvParam;

    while (1)
    {
        fSuccess = ReadFile(
            hPipe,
            pchRequest, 
            BUFSIZE*sizeof(TCHAR),
            &cbBytesRead, 
            NULL);     

        if (!fSuccess || cbBytesRead == 0) {
            if (GetLastError() == ERROR_BROKEN_PIPE) {
                printf("ClientThread: client disconnected, GetLastError=%d.\n", GetLastError());
            }
            else {
                printf("ClientThread ReadFile failed, GetLastError=%d.\n", GetLastError());
            }
            break;
        }

        printf("Client Request String:\"%ls\"\n", pchRequest);

        if (FAILED(StringCchCopy(pchReply, BUFSIZE, TEXT("Answer from server")))) {
            cbReplyBytes = 0;
            pchReply[0] = 0;
            printf("StringCchCopy failed, no outgoing message.\n");
            return (DWORD)-1;
        }
        cbReplyBytes = (lstrlen(pchReply) + 1)*sizeof(TCHAR);

        fSuccess = WriteFile(
            hPipe,        
            pchReply,    
            cbReplyBytes, 
            &cbWritten, 
            NULL);  

        if (!fSuccess || cbReplyBytes != cbWritten) {
            printf("ClientThread WriteFile failed, GetLastError=%d.\n", GetLastError());
            break;
        }
    }

    FlushFileBuffers(hPipe);
    DisconnectNamedPipe(hPipe);
    CloseHandle(hPipe);

    HeapFree(hHeap, 0, pchRequest);
    HeapFree(hHeap, 0, pchReply);

    printf("ClientThread exitting.\n");
    return 1;
}
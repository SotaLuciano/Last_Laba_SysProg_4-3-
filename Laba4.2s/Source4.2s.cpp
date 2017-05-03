#include <windows.h> 
#include <stdio.h> 
#include <tchar.h>
#include <strsafe.h>

#define BUFSIZE 4096
//Іменований канал (сервер)
int main()
{
    BOOL fConnected = FALSE;
    HANDLE hPipe = INVALID_HANDLE_VALUE;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");
    DWORD cbBytesRead = 0, cbReplyBytes = 0, cbWritten = 0;
    BOOL fSuccess = FALSE;

    while (1) {
        printf("\nPipe Server: Awaiting client connection on %ls\n", lpszPipename);
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
            printf("CreateNamedPipe failed, GetLastError = %d.\n", GetLastError());
            return -1;
        }

        fConnected = ConnectNamedPipe(hPipe, NULL) ? TRUE : GetLastError() == ERROR_PIPE_CONNECTED;

        if (fConnected) {
            printf("Client connected, receiving and processing messages.\n");
            TCHAR  pchRequest[BUFSIZE];
            TCHAR  pchReply[BUFSIZE];

            while (1) {
                fSuccess = ReadFile(
                    hPipe,        // handle to pipe 
                    pchRequest,    // buffer to receive data 
                    BUFSIZE*sizeof(TCHAR), // size of buffer 
                    &cbBytesRead, // number of bytes read 
                    NULL);        // not overlapped I/O 

                if (!fSuccess || cbBytesRead == 0) {
                    if (GetLastError() == ERROR_BROKEN_PIPE) {
                        printf("Client disconnected, GetLastError=%d.\n", GetLastError());
                    }
                    else {
                        printf("ReadFile failed, GetLastError=%d.\n", GetLastError());
                    }
                    break;
                }

                printf("Client Request String:\"%ls\"\n", pchRequest);
                StringCchCopy(pchReply, BUFSIZE, L"Answer from server");

                cbReplyBytes = (lstrlen(pchReply) + 1)*sizeof(TCHAR);
                fSuccess = WriteFile(
                    hPipe,
                    pchReply,
                    cbReplyBytes,
                    &cbWritten,
                    NULL);

                if (!fSuccess || cbReplyBytes != cbWritten) {
                    printf("WriteFile failed, GetLastError = %d.\n", GetLastError());
                    break;
                }
            }

            FlushFileBuffers(hPipe);
            DisconnectNamedPipe(hPipe);
            CloseHandle(hPipe);
        }
        else
            CloseHandle(hPipe);
    }

    return 0;
}
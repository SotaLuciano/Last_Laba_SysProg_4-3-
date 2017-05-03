#include <windows.h> 
#include <stdio.h>
#include <conio.h>

#define BUFSIZE 4096

//“¿ Œ∆ ¬» Œ–»—“Œ¬”™“‹—ﬂ ƒÀﬂ «¿¬ƒ¿ÕÕﬂ 4.3
//≤ÏÂÌÓ‚‡ÌËÈ Í‡Ì‡Î (ÍÎ≥∫ÌÚ)
int main() {
    HANDLE hPipe;
    LPTSTR lpvMessage = TEXT("Message from client.");
    TCHAR  chBuf[BUFSIZE];
    BOOL   fSuccess = FALSE;
    DWORD  cbRead, cbToWrite, cbWritten, dwMode;
    LPTSTR lpszPipename = TEXT("\\\\.\\pipe\\mynamedpipe");

    // Try to open a named pipe; wait for it, if necessary. 

    while (1) {
        hPipe = CreateFile(
            lpszPipename,
            GENERIC_READ | 
            GENERIC_WRITE,
            0, 
            NULL, 
            OPEN_EXISTING, 
            0,  
            NULL); 

        if (hPipe != INVALID_HANDLE_VALUE)
            break;

        if (GetLastError() != ERROR_PIPE_BUSY) {
            printf("Could not open pipe. GLE=%d\n", GetLastError());
            return -1;
        }

        if (!WaitNamedPipe(lpszPipename, 20000))  {
            puts("Could not open pipe: 20 second wait timed out.");
            return -1;
        }
    }

    dwMode = PIPE_READMODE_MESSAGE;
    fSuccess = SetNamedPipeHandleState(
        hPipe,
        &dwMode,
        NULL,
        NULL);

    if (!fSuccess) {
        printf("SetNamedPipeHandleState failed. GetLastError=%d\n", GetLastError());
        return -1;
    }

    cbToWrite = (lstrlen(lpvMessage) + 1)*sizeof(TCHAR);

    printf("Sending %d byte message: \"%ls\"\n", cbToWrite, lpvMessage);

    fSuccess = WriteFile(
        hPipe,
        lpvMessage, 
        cbToWrite, 
        &cbWritten,
        NULL); 

    if (!fSuccess)  {
        printf("WriteFile to pipe failed. GetLastError=%d\n", GetLastError());
        return -1;
    }

    printf("\nMessage sent to server, receiving reply:\n");

    do {
        // Read from the pipe. 

        fSuccess = ReadFile(
            hPipe,    // pipe handle 
            chBuf,    // buffer to receive reply 
            BUFSIZE*sizeof(TCHAR),  // size of buffer 
            &cbRead,  // number of bytes read 
            NULL);    // not overlapped 

        if (!fSuccess && GetLastError() != ERROR_MORE_DATA)
            break;

        printf("\"%ls\"\n", chBuf);
    } while (!fSuccess);  // repeat loop if ERROR_MORE_DATA 

    if (!fSuccess) {
        printf("ReadFile from pipe failed. GetLastError = %d\n", GetLastError());
        return -1;
    }

    puts("\n<End of message, press ENTER to terminate connection and exit>");
    _getch();

    CloseHandle(hPipe);

    return 0;
}
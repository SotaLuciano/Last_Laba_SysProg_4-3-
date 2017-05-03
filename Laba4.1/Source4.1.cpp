#include <windows.h>
#include <stdio.h>
#include <conio.h>
#include <tchar.h>

#define BUF_SIZE 4096
TCHAR szName[] = TEXT("MyFileMappingObject");
TCHAR szNameExisting[] = TEXT("MyFileMapingObjectExisting");
TCHAR szMsg[] = TEXT("Message from second process.");
TCHAR szFilePath[] = TEXT("D:\\Visual Studio\\System Programming\\Laba4\\Laba4\\text.txt");

int main() {
    HANDLE hMapFile, hMapExistingFile, hFile;
    LPCTSTR pBuf, pBufExisting;

    #pragma region Part 4.1.1
    puts("Process 2 started.\n");

    hMapFile = OpenFileMapping(
        FILE_MAP_ALL_ACCESS,
        FALSE,
        szName);

    if (!hMapFile) {
        printf("Could not open file mapping object (%d).\n", GetLastError());
        return 1;
    }

    pBuf = (LPTSTR)MapViewOfFile(hMapFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        BUF_SIZE);

    if (pBuf == NULL) {
        printf("Could not map view of file (%d.\n", GetLastError());
        CloseHandle(hMapFile);
        return 1;
    }

    printf("%ls\n", pBuf);
    _getch();
    #pragma endregion

    #pragma region Part 4.1.3
    puts("Getting access to existing file...");

    hFile = CreateFile(szFilePath,
        GENERIC_READ | GENERIC_WRITE,
        FILE_SHARE_READ | FILE_SHARE_WRITE,
        NULL,
        OPEN_ALWAYS,
        FILE_FLAG_RANDOM_ACCESS,
        NULL);

    if (GetLastError() != ERROR_ALREADY_EXISTS) {
        puts("Cannot open the file.");
        _getch();
        return 1;
    }

    puts("File opened.\nCreating file mapping...");

    hMapExistingFile = CreateFileMapping(
        hFile,
        NULL,
        PAGE_READWRITE,
        0,
        BUF_SIZE,
        szNameExisting);

    if (hMapExistingFile == NULL) {
        printf("Could not create file mapping object (%d).\n", GetLastError());
        CloseHandle(hFile);
        return 1;
    }

    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        printf("Already existing file mapping object with name %ls handled.\n", szNameExisting);
    }
    else {
        puts("File mapping object created.");
    }

    puts("Creating map view...");

    pBufExisting = (LPTSTR)MapViewOfFile(hMapExistingFile,
        FILE_MAP_ALL_ACCESS,
        0,
        0,
        BUF_SIZE);

    if (pBufExisting == NULL) {
        printf("Could not map view of file (%d).\n", GetLastError());
        CloseHandle(hMapExistingFile);
        CloseHandle(hFile);
        return 1;
    }

    puts("Map view created.");
	CopyMemory((PVOID)pBufExisting, szMsg, (_tcslen(szMsg) * sizeof(TCHAR)));
    _getch();
    #pragma endregion 

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);
    UnmapViewOfFile(pBufExisting);
    CloseHandle(hMapExistingFile);
    CloseHandle(hFile);
    return 0;
}
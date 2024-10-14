#include <windows.h>
#include <iostream>

#define SHARED_MEMORY_NAME L"Global\\MantleBlock"

VOID CheckCOMs(LPVOID pBuf) {
    std::cout << "Writing OKUM...\n";

    const char* userReady = "OKUM";
    memcpy(pBuf, userReady, strlen(userReady) + 1);

    std::cout << "Memory is ready.\n";
    Sleep(100);
}

VOID WriteToSharedMemory(LPVOID pBuf, const char* message) {
    memcpy(pBuf, message, strlen(message) + 1);
}

PCHAR ReadFromSharedMemory(LPVOID pBuf) {
    return (char*)pBuf;
}

VOID LinkSharedMemory(HANDLE hMapFile, LPVOID pBuf) {
    // Open the shared memory section
    hMapFile = OpenFileMapping(FILE_MAP_ALL_ACCESS, FALSE, SHARED_MEMORY_NAME);
    if (hMapFile == NULL) {
        std::cerr << "Failed to open shared memory. Error: " << GetLastError() << std::endl;
        return;
    }

    // Map the shared memory into the process's address space
    pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, 0);
    if (pBuf == NULL) {
        std::cerr << "Failed to map shared memory. Error: " << GetLastError() << std::endl;
        CloseHandle(hMapFile);
        return;
    }
}
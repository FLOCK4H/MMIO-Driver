#include <windows.h>
#include <iostream>
#include "utils.h"
#include "xmem.h"

HANDLE hMapFile;
LPVOID pBuf;
BOOLEAN attached = false;

VOID AttachDriverToProcess(const wchar_t* p_name) {
    DWORD pid = get_process_id(p_name);
    if (pid == 0) {
        std::cerr << "Notepad process not found!" << std::endl;
        UnmapViewOfFile(pBuf);
        CloseHandle(hMapFile);
        return;
    }

    char attachCommand[256] = { 0 };
    snprintf(attachCommand, sizeof(attachCommand), "ATTACH-%lu", pid);
    WriteToSharedMemory(pBuf, attachCommand);

    std::cout << "Sent ATTACH command to driver: " << attachCommand << std::endl;
    Sleep(2000);
    if (strcmp(ReadFromSharedMemory(pBuf), "OK") == 0) {
        std::cout << "Driver attached!" << std::endl;
    }
    else {
        std::cerr << "Failed to attach driver!" << std::endl;
    }
}

VOID ReadProcessMemory(long long address) {
    DWORD64 Raddress = address;
    char readCommand[256] = { 0 };
    snprintf(readCommand, sizeof(readCommand), "READ-%llx", (DWORD64)Raddress);
    WriteToSharedMemory(pBuf, readCommand);

    std::cout << "Sent READ command to driver: " << readCommand << std::endl;

    while (strncmp(ReadFromSharedMemory(pBuf), "VALUE-", 6) != 0) {
        Sleep(200);
    }

    char* Rresponse = ReadFromSharedMemory(pBuf);
    PCHAR value = Rresponse + 6;
    std::cout << "Driver responded with: " << value << std::endl;
}

VOID WriteProcessMemory(long long address, const char* value) {
    DWORD64 Waddress = address;
    char writeCommand[256] = { 0 };
    snprintf(writeCommand, sizeof(writeCommand), "WRITE-%llx-%s", (DWORD64)Waddress, value);
    WriteToSharedMemory(pBuf, writeCommand);
}
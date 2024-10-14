#include <windows.h>
#include <iostream>
#include <cstring>
#include <TlHelp32.h>

#include "xmem.h"
#include "cDriver.h"

int main() {
    LinkSharedMemory(hMapFile, pBuf);
    CheckCOMs(pBuf);
    AttachDriverToProcess(L"notepad.exe");

	// ReadProcessMemory(0x17CAC8C7954); // 64-bit address
	// WriteProcessMemory(0x17CAC8C7954, "0x1234");

    UnmapViewOfFile(pBuf);
    CloseHandle(hMapFile);

    std::cin.get();
    return 0;
}
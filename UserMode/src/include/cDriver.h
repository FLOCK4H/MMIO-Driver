#pragma once

#include <windows.h>
#include <iostream>

extern HANDLE hMapFile;
extern LPVOID pBuf;    
extern BOOLEAN attached;
VOID AttachDriverToProcess(const wchar_t* p_name);
VOID ReadProcessMemory(long long address);
VOID WriteProcessMemory(long long address, const char* value);
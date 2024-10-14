#pragma once

#include <windows.h>
#include <iostream>

VOID CheckCOMs(LPVOID pBuf);
VOID WriteToSharedMemory(LPVOID pBuf, const char* message);
PCHAR ReadFromSharedMemory(LPVOID pBuf);
VOID LinkSharedMemory(HANDLE hMapFile, LPVOID pBuf);
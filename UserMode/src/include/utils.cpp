#include <windows.h>
#include <iostream>
#include <TlHelp32.h>

DWORD get_process_id(const std::wstring& process_name) // func from web
{
    DWORD process_id = 0;
    HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);
    if (snapshot == INVALID_HANDLE_VALUE)
        return process_id;

    PROCESSENTRY32W entry = {};
    entry.dwSize = sizeof(entry);

    if (Process32FirstW(snapshot, &entry)) {
        if (_wcsicmp(process_name.c_str(), entry.szExeFile) == 0)
            process_id = entry.th32ProcessID;
        else {
            while (Process32NextW(snapshot, &entry)) {
                if (_wcsicmp(process_name.c_str(), entry.szExeFile) == 0) {
                    process_id = entry.th32ProcessID;
                    break;
                }
            }
        }
    }

    CloseHandle(snapshot);
    return process_id;
}
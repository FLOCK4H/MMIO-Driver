#pragma once

#include <ntddk.h>

namespace mantles {
	NTSTATUS AttachToProcess(HANDLE process_id);
	NTSTATUS ReadFromProc(PVOID target, PVOID buffer, SIZE_T size, SIZE_T* return_size);
	NTSTATUS WriteToProc(PVOID target, PVOID value, SIZE_T size, SIZE_T* return_size);
}

NTSTATUS RtlCharToInteger64(const CHAR* str, ULONG base, DWORD64* value);

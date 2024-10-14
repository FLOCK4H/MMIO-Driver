#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntddk.h>
#include <stdlib.h>

#include "utils.h"

extern "C" {
    NTKERNELAPI NTSTATUS MmCopyVirtualMemory(PEPROCESS SourceProcess, PVOID SourceAddress,
        PEPROCESS TargetProcess, PVOID TargetAddress,
        SIZE_T BufferSize, KPROCESSOR_MODE PreviousMode,
        PSIZE_T ReturnSize);
}

namespace mantles {
    static PEPROCESS target_process = nullptr;
    NTSTATUS status;

    NTSTATUS AttachToProcess(HANDLE process_id) {
        status = PsLookupProcessByProcessId(process_id, &target_process);
        if (!NT_SUCCESS(status)) {
            debug_print("[MANTLE] Failed to attach to process.\n");
        }

        return status;
    }

    NTSTATUS ReadFromProc(PVOID target, PVOID buffer, SIZE_T size, SIZE_T* return_size) {
        if (target_process != nullptr) {
            // Print correct address and PEPROCESS structure for debugging
            debug_print("Reading from address: %llx\n", (DWORD64)target);
            debug_print("Target process PEPROCESS: %llx\n", (DWORD64)target_process);

            PEPROCESS current_process = PsGetCurrentProcess();
            NTSTATUS status = MmCopyVirtualMemory(
                target_process,  // Source process
                target,          // Source address
                current_process, // Target process (current process)
                buffer,          // Target buffer (SharedMemory)
                size,            // Buffer size
                KernelMode,      // Operation mode
                return_size      // Return size (number of bytes successfully read)
            );

            if (!NT_SUCCESS(status)) {
                debug_print("[MANTLE] MmCopyVirtualMemory failed with status: 0x%x\n", status);
            }
            return status;
        }
        else {
            debug_print("[MANTLE] No target process attached.\n");
            return STATUS_INVALID_PARAMETER;
        }
    }

    NTSTATUS WriteToProc(PVOID target, PVOID value, SIZE_T size, SIZE_T* return_size) {
        if (target_process != nullptr) {
            debug_print("Writing to address: %llx\n", (DWORD64)target);
            debug_print_str("Value: %s\n", (PCSTR)value);
            debug_print("Target process PEPROCESS: %llx\n", (DWORD64)target_process);

            PEPROCESS current_process = PsGetCurrentProcess();
            NTSTATUS status = MmCopyVirtualMemory(
                current_process, // Source process (current process)
                value,
                target_process,  // Target process
                target,          // Target address
                size,            // Buffer size
                KernelMode,      // Operation mode
                return_size      // Return size (number of bytes successfully written)
            );

            if (!NT_SUCCESS(status)) {
                debug_print("[MANTLE] MmCopyVirtualMemory failed with status: 0x%x\n", status);
            }
            return status;
        }
        else {
            debug_print("[MANTLE] No target process attached.\n");
            return STATUS_INVALID_PARAMETER;
        }
    }
}

NTSTATUS RtlCharToInteger64(const CHAR* str, ULONG base, DWORD64* value) {
    *value = 0;
    while (*str) {
        CHAR c = *str++;
        ULONG digit = 0;

        if (c >= '0' && c <= '9') {
            digit = c - '0';
        }
        else if (c >= 'A' && c <= 'F') {
            digit = c - 'A' + 10;
        }
        else if (c >= 'a' && c <= 'f') {
            digit = c - 'a' + 10;
        }
        else {
            return STATUS_INVALID_PARAMETER;
        }

        if (digit >= base) {
            return STATUS_INVALID_PARAMETER;
        }

        *value = *value * base + digit;
    }
    return STATUS_SUCCESS;
}
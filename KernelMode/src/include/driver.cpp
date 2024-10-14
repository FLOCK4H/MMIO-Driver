#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntddk.h>
#include <stdlib.h>
#include "utils.h"
#include "mantles.h"

PVOID SharedMemory = NULL;
HANDLE SharedMemoryHandle = NULL;
HANDLE PollingThreadHandle = NULL;

namespace driver {
    VOID AttachToProcess(char* sharedMemoryData) {
        debug_print("User-mode application signaled 'ATTACH'!\n");

        CHAR* pidString = sharedMemoryData + 7;
        ULONG pid = 0;

        NTSTATUS status = RtlCharToInteger(pidString, 10, &pid);
        if (!NT_SUCCESS(status)) {
            debug_print("Failed to parse PID from the string.\n");

            const char* readyMessage = "FAIL";
            RtlCopyMemory(SharedMemory, readyMessage, strlen(readyMessage) + 1);
            debug_print("Driver wrote 'FAIL' to shared memory.\n");
        }
        else {
            debug_print("Parsed PID successfully.\n");
            mantles::AttachToProcess((HANDLE)(ULONG_PTR)pid);
            const char* readyMessage = "OK";
            RtlCopyMemory(SharedMemory, readyMessage, strlen(readyMessage) + 1);
            debug_print("Driver wrote 'OK' to shared memory.\n");
        }
    }

    VOID ReadFromProcess(char* sharedMemoryData) {
        CHAR* addressString = sharedMemoryData + 5;
        DWORD64 address = 0;

        NTSTATUS status = RtlCharToInteger64(addressString, 16, &address); // We need custom func for 64bit programs
        if (!NT_SUCCESS(status)) {
            debug_print("Failed to parse address from the string.\n");
            const char* failMessage = "FAIL";
            RtlCopyMemory(SharedMemory, failMessage, strlen(failMessage) + 1);
        }
        else {
            SIZE_T returnSize = 0;
            ULONG value = 0;

            status = mantles::ReadFromProc((PVOID)(DWORD64)address, &value, sizeof(value), &returnSize);
            if (!NT_SUCCESS(status)) {
                debug_print("Failed to read from process.\n");
                const char* failMessage = "FAIL";
                RtlCopyMemory(SharedMemory, failMessage, strlen(failMessage) + 1);
            }
            else {
                CHAR valueString[64] = { 0 };
                RtlStringCbPrintfA(valueString, sizeof(valueString), "VALUE-%08X", value);  // Format the value as hex

                RtlCopyMemory(SharedMemory, valueString, strlen(valueString) + 1);
                debug_print("Driver wrote the value to shared memory: %08X\n", value);
            }
        }
    }

    VOID WriteToProcess(char* sharedMemoryData) {
        CHAR* addressString = sharedMemoryData + 6;
        CHAR* valueString = strstr(addressString, "-");

        if (valueString == NULL) {
            debug_print("Failed to parse value from the string.\n");
            const char* failMessage = "FAIL";
            RtlCopyMemory(SharedMemory, failMessage, strlen(failMessage) + 1);
            debug_print("Driver wrote 'FAIL' to shared memory.\n");
            return;
        }

        *valueString = '\0';
        valueString++;

        debug_print_str("Value string: %s\n", valueString);

        DWORD64 address = 0;
        NTSTATUS status = RtlCharToInteger64(addressString, 16, &address); // Parse address

        if (!NT_SUCCESS(status)) {
            debug_print("Failed to parse address from the string.\n");
            const char* failMessage = "FAIL";
            RtlCopyMemory(SharedMemory, failMessage, strlen(failMessage) + 1);
            debug_print("Driver wrote 'FAIL' to shared memory.\n");
        }
        else {
            debug_print("Parsed address successfully: %llx\n", address);
            SIZE_T returnSize = 0;

            // Check if value is numeric or string
            if (isdigit(valueString[0])) {
                ULONG numericValue = atoi(valueString);
                status = mantles::WriteToProc((PVOID)(DWORD64)address, &numericValue, sizeof(numericValue), &returnSize);
            }
            else {
                SIZE_T valueLength = strlen(valueString);
                status = mantles::WriteToProc((PVOID)(DWORD64)address, valueString, valueLength + 1, &returnSize);
            }

            if (!NT_SUCCESS(status)) {
                debug_print("Failed to write to process.\n");
                const char* failMessage = "FAIL";
                RtlCopyMemory(SharedMemory, failMessage, strlen(failMessage) + 1);
                debug_print("Driver wrote 'FAIL' to shared memory.\n");
            }
            else {
                const char* successMessage = "SUCCESS";
                RtlCopyMemory(SharedMemory, successMessage, strlen(successMessage) + 1);
                debug_print("Driver wrote 'SUCCESS' to shared memory.\n");
            }
        }
    }

    VOID ContinousCOMChannel() {
        while (TRUE) {
            debug_print("> ContinousCOMChannel");
            CHAR* sharedMemoryData = (CHAR*)SharedMemory;
            if (strncmp(sharedMemoryData, "ATTACH-", 7) == 0) {
				AttachToProcess(sharedMemoryData);
            }
            else if (strncmp(sharedMemoryData, "READ-", 5) == 0) {
				ReadFromProcess(sharedMemoryData);
            }
            else if (strncmp(sharedMemoryData, "WRITE-", 6) == 0) {
				WriteToProcess(sharedMemoryData);
            }

            LARGE_INTEGER delay;
            delay.QuadPart = -10 * 1000 * 100;  // 100ms delay
            KeDelayExecutionThread(KernelMode, FALSE, &delay);
        }
    }

    VOID PollSharedMemory() {
        ContinousCOMChannel();
    }
}

NTSTATUS CreateSharedMemory(SIZE_T Size) {
    NTSTATUS status;
    OBJECT_ATTRIBUTES objAttr;
    UNICODE_STRING sectionName;
    LARGE_INTEGER maxSize;
    maxSize.QuadPart = Size;

    RtlInitUnicodeString(&sectionName, L"\\BaseNamedObjects\\Global\\MantleBlock");

    debug_print("SecDesc INIT %x\n");

    SECURITY_DESCRIPTOR securityDescriptor;
    status = RtlCreateSecurityDescriptor(&securityDescriptor, SECURITY_DESCRIPTOR_REVISION);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to create security descriptor: %x\n");
        return status;
    }

    debug_print("DACL INIT %x\n");

    status = RtlSetDaclSecurityDescriptor(&securityDescriptor, TRUE, NULL, FALSE);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to set DACL: %x\n");
        return status;
    }

    InitializeObjectAttributes(&objAttr, &sectionName, OBJ_CASE_INSENSITIVE, NULL, &securityDescriptor);
    debug_print("Initialized.\n");

    status = ZwCreateSection(&SharedMemoryHandle, SECTION_ALL_ACCESS, &objAttr, &maxSize, PAGE_READWRITE, SEC_COMMIT, NULL);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to create memory section: %x\n");
        return status;
    }

    SIZE_T viewSize = 0;
    status = ZwMapViewOfSection(SharedMemoryHandle, ZwCurrentProcess(), &SharedMemory, 0, 0, NULL, &viewSize, ViewUnmap, 0, PAGE_READWRITE);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to map shared memory: %x\n");
        return status;
    }

    debug_print("Trying to write test data to the shared memory...\n");
    const char* readyMessage = "OKKM";
    RtlCopyMemory(SharedMemory, readyMessage, strlen(readyMessage) + 1);
    debug_print("Driver wrote 'OKKM' to shared memory...\n");

    return STATUS_SUCCESS;
}

VOID PollingThreadRoutine(PVOID Context) {
    UNREFERENCED_PARAMETER(Context);
    PVOID Thread = KeGetCurrentThread();

    NTSTATUS status = CreateSharedMemory(0x1000);  // 4 KB
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to create shared memory inside thread: %x\n");
        PsTerminateSystemThread(status);
        return;
    }
    
    driver::PollSharedMemory();

    PsTerminateSystemThread(STATUS_SUCCESS);
}
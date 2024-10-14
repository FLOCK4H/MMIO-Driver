#include <ntifs.h>
#include <ntstrsafe.h>
#include <ntddk.h>
#include <stdlib.h>
#include "driver.h"
#include "utils.h"

#define DEVICE_NAME L"\\Device\\Mantle"
#define SYMLINK_NAME L"\\DosDevices\\Mantle"

extern "C" {
    NTKERNELAPI NTSTATUS IoCreateDriver(PUNICODE_STRING DriverName,
        PDRIVER_INITIALIZE InitializationFunction);
}

VOID DriverUnload(PDRIVER_OBJECT DriverObject) {
    UNICODE_STRING symbolicLinkName;
    RtlInitUnicodeString(&symbolicLinkName, SYMLINK_NAME);
    IoDeleteSymbolicLink(&symbolicLinkName);

    IoDeleteDevice(DriverObject->DeviceObject);

    // Properly terminate the polling thread
    if (PollingThreadHandle) {
        NTSTATUS status = KeWaitForSingleObject(PollingThreadHandle, Executive, KernelMode, FALSE, NULL);
        if (!NT_SUCCESS(status)) {
            debug_print("Failed to wait for polling thread termination: %x");
        }
        ZwClose(PollingThreadHandle);
    }

    // Clean up shared memory resources
    if (SharedMemory) {
        ZwUnmapViewOfSection(ZwCurrentProcess(), SharedMemory);
    }
    if (SharedMemoryHandle) {
        ZwClose(SharedMemoryHandle);
    }
}

NTSTATUS CreateDeviceAndLink(PDRIVER_OBJECT DriverObject, PUNICODE_STRING registry_path) {
    UNREFERENCED_PARAMETER(registry_path);

    NTSTATUS status;
    UNICODE_STRING deviceName;
    UNICODE_STRING symbolicLinkName;
    PDEVICE_OBJECT deviceObject = NULL;

    DriverObject->DriverUnload = DriverUnload;

    RtlInitUnicodeString(&deviceName, DEVICE_NAME);
    status = IoCreateDevice(DriverObject, 0, &deviceName, FILE_DEVICE_UNKNOWN, FILE_DEVICE_SECURE_OPEN, FALSE, &deviceObject);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to create device.\n");
        return status;
    }

    RtlInitUnicodeString(&symbolicLinkName, SYMLINK_NAME);
    status = IoCreateSymbolicLink(&symbolicLinkName, &deviceName);
    if (!NT_SUCCESS(status)) {
        IoDeleteDevice(deviceObject);
        debug_print("Failed to create symbolic link.\n");
        return status;
    }

    deviceObject->Flags |= DO_BUFFERED_IO;
    deviceObject->Flags &= ~DO_DEVICE_INITIALIZING;

    // Polling system thread
    HANDLE threadHandle;
    status = PsCreateSystemThread(&threadHandle, THREAD_ALL_ACCESS, NULL, NULL, NULL, PollingThreadRoutine, NULL);
    if (!NT_SUCCESS(status)) {
        debug_print("Failed to create polling thread\n");
        return status;
    }

    PollingThreadHandle = threadHandle;

    return STATUS_SUCCESS;
}

extern "C"
NTSTATUS DriverEntry(PDRIVER_OBJECT DriverObject, PUNICODE_STRING RegistryPath) {
    debug_print("[MANTLE] Protocol working!\n");

    UNICODE_STRING driver_name;
    RtlInitUnicodeString(&driver_name, L"\\Driver\\Mantle");

    return IoCreateDriver(&driver_name, &CreateDeviceAndLink);
}
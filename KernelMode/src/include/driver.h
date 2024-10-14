// driver.h

#pragma once
#include <ntddk.h>

extern PVOID SharedMemory;
extern HANDLE SharedMemoryHandle;
extern HANDLE PollingThreadHandle;

void PollingThreadRoutine(PVOID Context);

namespace driver {
    void PollSharedMemory();
}
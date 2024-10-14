#include <ntifs.h>

void debug_print(PCSTR format, DWORD64 value = 0) {
    if (value) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, value));
    }
    else {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format));
    }
}

void debug_print_str(PCSTR format, PCSTR text) {
    if (text) {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format, text));
    }
    else {
        KdPrintEx((DPFLTR_IHVDRIVER_ID, DPFLTR_INFO_LEVEL, format));
    }
}
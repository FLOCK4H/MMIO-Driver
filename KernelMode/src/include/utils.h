#pragma once

#include <ntddk.h>

void debug_print(PCSTR format, DWORD64 value = 0);
void debug_print_str(PCSTR format, PCSTR text);
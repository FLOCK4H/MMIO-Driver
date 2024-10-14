# Overview

A Memory Mapped Input/Output driver for stealth communication between UserMode and KernelMode that handles memory read/write processes.

> [!CAUTION]
> The driver is UNSAFE, loading it incorrectly may result in BSOD.

Language: C++ 20
Platform: Windows

# Usage

The driver was made for communication between Kernel and User processes with a common ability to read/write memory from or to a process.
It was tested on Windows10-19045.

Find your way of mapping the driver, or sign it.

For development purposes:

1. Run WinDbg
2. Connect it to your VM's kernel
3. Load the driver (e.g. using <a href="https://github.com/TheCruZ/kdmapper">KdMapper</a>)
4. Run the UserMode application
5. Everything should be 'OK' in the terminals, then it works.

You must actually perform some transactions between UserMode and KernelMode for the driver to have a purpose, the driver itself does not do **ANYTHING**.

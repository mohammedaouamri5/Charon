# ~/.gdbinit-override: add add-auto-load-safe-path ... if gdb warns
# How to use:
#   terminal 1: cmake --build build && cmake --build build --target debug
#   terminal 2: gdb -x .gdbinit

# we are a 32-bit target, host gdb is 64-bit
set architecture i386

# less noise between stops
set pagination off

# connect to QEMU's stub (started with -s -S)
target remote localhost:1234

# symbols from the kernel binary
file build/charon.bin

# break at the C entry point, then let the bootloader + boot.s run
hbreak _start
hbreak kernel_main
continue

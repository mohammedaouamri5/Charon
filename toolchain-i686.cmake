set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR i686)

set(CMAKE_C_COMPILER   i686-elf-gcc)
set(CMAKE_CXX_COMPILER i686-elf-g++)
set(CMAKE_ASM_COMPILER i686-elf-gcc)

# there's no OS/libc to link a test program against
set(CMAKE_TRY_COMPILE_TARGET_TYPE STATIC_LIBRARY)

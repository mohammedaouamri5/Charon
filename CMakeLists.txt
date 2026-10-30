cmake_minimum_required(VERSION 3.20)
project(Charon LANGUAGES C CXX ASM)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)   # writes build/compile_commands.json

# every .cpp/.s under src/ is picked up; new files are noticed on rebuild
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    ${CMAKE_SOURCE_DIR}/src/*.cpp
    ${CMAKE_SOURCE_DIR}/src/*.s)

add_executable(charon ${SOURCES})
set_target_properties(charon PROPERTIES OUTPUT_NAME charon SUFFIX ".bin")

target_include_directories(charon PRIVATE src)

target_compile_options(charon PRIVATE
    -ffreestanding -Wall -Wextra
    $<$<COMPILE_LANGUAGE:CXX>:-fno-exceptions>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-rtti>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-threadsafe-statics>
    $<$<COMPILE_LANGUAGE:CXX>:-fno-use-cxa-atexit>
)

target_link_options(charon PRIVATE
    -ffreestanding -nostdlib
    -Wl,--no-warn-rwx-segments
    -T ${CMAKE_SOURCE_DIR}/link.ld
)
set_target_properties(charon PROPERTIES LINK_DEPENDS ${CMAKE_SOURCE_DIR}/link.ld)

target_link_libraries(charon PRIVATE gcc)   # -lgcc, placed after the objects

# ---- ISO: copies iso/ (with boot/grub/grub.cfg) and adds the kernel ----
add_custom_target(iso
    COMMAND ${CMAKE_COMMAND} -E rm -rf ${CMAKE_BINARY_DIR}/isodir
    COMMAND ${CMAKE_COMMAND} -E copy_directory ${CMAKE_SOURCE_DIR}/iso ${CMAKE_BINARY_DIR}/isodir
    COMMAND ${CMAKE_COMMAND} -E copy $<TARGET_FILE:charon> ${CMAKE_BINARY_DIR}/isodir/boot/charon.bin
    COMMAND grub-mkrescue -o ${CMAKE_BINARY_DIR}/charon.iso ${CMAKE_BINARY_DIR}/isodir
    DEPENDS charon
    COMMENT "Building charon.iso"
)

# ---- run targets ----
add_custom_target(run
    COMMAND qemu-system-i386 -m 128M -kernel $<TARGET_FILE:charon>
    DEPENDS charon USES_TERMINAL)

add_custom_target(run-iso
    COMMAND qemu-system-i386 -m 128M -cdrom ${CMAKE_BINARY_DIR}/charon.iso
    DEPENDS iso USES_TERMINAL)

add_custom_target(check-multiboot
    COMMAND grub-file --is-x86-multiboot $<TARGET_FILE:charon>
    COMMAND ${CMAKE_COMMAND} -E echo "multiboot header OK"
    DEPENDS charon)

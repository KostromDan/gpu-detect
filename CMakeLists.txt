cmake_minimum_required(VERSION 3.15)
project(gpu-detect LANGUAGES CXX)

# ───────────── General Settings ─────────────
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(gpu-detect gpu-detect.cpp)

# Find UPX (Ultimate Packer for eXecutables)
find_program(UPX_EXECUTABLE upx)

# System DXGI / D3D12 / COM import libraries
target_link_libraries(gpu-detect PRIVATE dxgi d3d12 ole32)

# Add post-build UPX compression if found
if(UPX_EXECUTABLE)
    add_custom_command(TARGET gpu-detect POST_BUILD
        COMMAND ${UPX_EXECUTABLE} --best --lzma "$<TARGET_FILE:gpu-detect>"
        COMMENT "Compressing executable with UPX")
    message(STATUS "UPX compression enabled (${UPX_EXECUTABLE})")
else()
    message(STATUS "UPX not found, executable compression disabled")
endif()

# ───────────── MinGW / size-optimised, one-file build ─────────────
if(MINGW)
    # ----------------------------------------------------------
    # 1. Compiler flags (size first)
    # ----------------------------------------------------------
    target_compile_options(gpu-detect PRIVATE
            -Os                         # optimise for size
            -ffunction-sections
            -fdata-sections
            -fno-exceptions             # strip C++ EH
            -fno-rtti                   # strip RTTI
            -fno-unwind-tables
            -fno-asynchronous-unwind-tables
            -s)                         # strip symbols in .o files

    # ----------------------------------------------------------
    # 2. Linker flags (static, one EXE, discard unused)
    # ----------------------------------------------------------
    target_link_options(gpu-detect PRIVATE
            -static                     # keep everything inside the EXE
            -static-libgcc
            -static-libstdc++
            -lwinpthread                # static winpthread.a
            -Wl,--gc-sections           # drop unused sections
            -s)                         # strip symbol table
endif()
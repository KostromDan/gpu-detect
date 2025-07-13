// gpu-detect.cpp : prints each adapter as INTEGRATED or DEDICATED
#include "gpu-detect-shared.h"
#include <cstdio>

int main() {
    // Create a buffer to capture all output - using heap instead of stack
    auto hBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1 << 16); // 64 KiB
    char* buffer = static_cast<char*>(hBuf);

    // Get the GPU detection output
    DetectGPUs(buffer, 1 << 16);
    
    // Print the result
    printf("%s", buffer);
    
    // Free heap-allocated buffer
    HeapFree(GetProcessHeap(), 0, buffer);

    return 0;
}

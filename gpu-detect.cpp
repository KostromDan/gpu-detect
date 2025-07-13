// gpu-detect.cpp : prints each adapter as INTEGRATED or DEDICATED
#include "gpu-detect-shared.h"
#include <cstdio>

int main() {
    // Create a buffer to capture all output - using heap instead of stack
    const size_t bufferSize = 1 << 20; // 1MB = 1,048,576 bytes
    auto hBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
    char *buffer = static_cast<char *>(hBuf);

    // Get the GPU detection output
    DetectGPUs(buffer, bufferSize);

    // Print the result
    printf("%s", buffer);

    // Free heap-allocated buffer
    HeapFree(GetProcessHeap(), 0, buffer);

    return 0;
}

// gpu-detect.cpp : prints each adapter as INTEGRATED or DEDICATED
#include "gpu-detect-shared.h"
#include <cstdio>

int main() {
    // Create a buffer to capture all output
    char buffer[4096] = {0};
    
    // Get the GPU detection output
    DetectGPUs(buffer, sizeof(buffer));
    
    // Print the result
    printf("%s", buffer);
    
    return 0;
}

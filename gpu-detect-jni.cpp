// gpu-detect-jni.cpp
#include "dev_kostromdan_mods_crash_assistant_app_utils_gpu_DirectXGPUDetector.h"
#include "gpu-detect-shared.h"

// -----------------------------------------------------------------------------
// 1) Export a new symbol DetectGPUsFFM that wraps the existing static
//    DetectGPUs(buffer, size) from gpu-detect-shared.h.
// -----------------------------------------------------------------------------
#ifdef __cplusplus
extern "C"
#endif
__declspec(dllexport)
char* DetectGPUsFFM(char* buffer, size_t bufferSize) {
    // Check for invalid input
    if (!buffer || bufferSize < 1) return nullptr;

    // calls the static DetectGPUs(…) implementation in gpu-detect-shared.h
    return DetectGPUs(buffer, bufferSize);
}

// -----------------------------------------------------------------------------
// 2) JNI bridge calls the internal static DetectGPUs.
// -----------------------------------------------------------------------------
JNIEXPORT jstring JNICALL
Java_dev_kostromdan_mods_crash_1assistant_app_utils_gpu_DirectXGPUDetector_getSerialisedGPUs(
    JNIEnv *env, jclass /*cls*/)
{
    // Use heap instead of stack - 64 KiB buffer
    auto hBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, 1 << 16);
    char* buffer = static_cast<char*>(hBuf);

    // Get the GPU detection output
    DetectGPUs(buffer, 1 << 16);    // static fn in gpu-detect-shared.h

    // Check if result is too large for JNI (max size is 65534 chars + null terminator)
    size_t resultLen = strlen(buffer);
    if (resultLen >= 65535) {
        // Calculate how many characters need to be truncated
        size_t excessChars = resultLen - 65534;
        // Create a prefix message about truncation
        char prefix[128];
        int prefixLen = snprintf(prefix, sizeof(prefix), "gpu-detect: result too large and was truncated by %zu chars\n", excessChars);

        // Make room for the prefix by shifting data
        if (prefixLen > 0) {
            // Move the buffer content to make room for the prefix
            // Calculate how much of the original buffer to keep (excluding the truncated part)
            size_t keepLen = 65534 - prefixLen;
            // Shift the content we want to keep to make room for the prefix
            memmove(buffer + prefixLen, buffer, keepLen);
            // Copy the prefix to the beginning of the buffer
            memcpy(buffer, prefix, prefixLen);
            // Ensure null termination
            buffer[prefixLen + keepLen] = '\0';
        }
    }

    // Convert to Java string and free buffer
    jstring result = env->NewStringUTF(buffer);
    HeapFree(GetProcessHeap(), 0, buffer);
    return result;
}

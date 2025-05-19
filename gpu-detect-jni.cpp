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
    // calls the static DetectGPUs(…) implementation in gpu-detect-shared.h
    return DetectGPUs(buffer, bufferSize);
}

// -----------------------------------------------------------------------------
// 2) JNI bridge remains unchanged—calls the internal static DetectGPUs.
// -----------------------------------------------------------------------------
JNIEXPORT jstring JNICALL
Java_dev_kostromdan_mods_crash_1assistant_app_utils_gpu_DirectXGPUDetector_getSerialisedGPUs(
    JNIEnv *env, jclass /*cls*/)
{
    char buffer[4096] = {0};
    DetectGPUs(buffer, sizeof(buffer));    // static fn in gpu-detect-shared.h
    return env->NewStringUTF(buffer);
}

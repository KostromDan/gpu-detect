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
char *DetectGPUsFFM(char *buffer, size_t bufferSize) {
    // Check for invalid input
    if (!buffer || bufferSize < 1) return nullptr;

    // calls the static DetectGPUs(…) implementation in gpu-detect-shared.h
    return DetectGPUs(buffer, bufferSize);
}

// -----------------------------------------------------------------------------
// 2) JNI bridge calls the internal static DetectGPUs.
// -----------------------------------------------------------------------------
extern "C" JNIEXPORT jbyteArray JNICALL
Java_dev_kostromdan_mods_crash_1assistant_app_utils_gpu_DirectXGPUDetector_getNativeSerialisedGPUs(
    JNIEnv *env, jclass /*cls*/) {
    /* --- constant error texts (ASCII/UTF-8) ----------------------------- */
    static constexpr const char kAllocFailMsg[] = "ERROR: native heap allocation failed";
    static constexpr const char kByteArrFailMsg[] = "ERROR: JVM byte-array allocation failed";

    /* --- allocate the native scratch buffer ----------------------------- */
    const size_t bufferSize = 1 << 20; // 1 MB
    auto hBuf = HeapAlloc(GetProcessHeap(), HEAP_ZERO_MEMORY, bufferSize);
    char *buffer = static_cast<char *>(hBuf);

    /* --------------------------------------------------------------------
     * If the native allocation fails we still try to create a Java byte[]
     * and fill it with the explanatory text so the caller can handle the
     * situation gracefully.
     * ------------------------------------------------------------------ */
    if (!buffer) {
        const jsize msgLen = static_cast<jsize>(sizeof(kAllocFailMsg) - 1); // drop trailing NUL
        jbyteArray errArr = env->NewByteArray(msgLen);
        if (errArr)
            env->SetByteArrayRegion(errArr, 0, msgLen,
                                    reinterpret_cast<const jbyte *>(kAllocFailMsg));
        return errArr; // may still be nullptr
    }

    /* --- normal GPU detection path -------------------------------------- */
    DetectGPUs(buffer, bufferSize);

    const size_t resultLen = ::strlen(buffer);
    jbyteArray byteArr = env->NewByteArray(static_cast<jsize>(resultLen));

    /* --------------------------------------------------------------------
     * If the JVM fails to allocate the result array we again fall back
     * to returning a tiny array with an error message.
     * ------------------------------------------------------------------ */
    if (!byteArr) {
        HeapFree(GetProcessHeap(), 0, buffer);

        const jsize msgLen = static_cast<jsize>(sizeof(kByteArrFailMsg) - 1);
        jbyteArray errArr = env->NewByteArray(msgLen);
        if (errArr)
            env->SetByteArrayRegion(errArr, 0, msgLen,
                                    reinterpret_cast<const jbyte *>(kByteArrFailMsg));
        return errArr;
    }

    /* --- copy data & clean-up ------------------------------------------- */
    env->SetByteArrayRegion(byteArr, 0, static_cast<jsize>(resultLen),
                            reinterpret_cast<const jbyte *>(buffer));

    HeapFree(GetProcessHeap(), 0, buffer);
    return byteArr;
}

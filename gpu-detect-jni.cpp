// gpu-detect-jni.cpp: JNI bridge for GPU detection
#include "dev_kostromdan_mods_crash_assistant_app_utils_gpu_DirectXGPUDetector.h"
#include "gpu-detect-shared.h"

/*
 * Class:     dev_kostromdan_mods_crash_assistant_app_utils_gpu_DirectXGPUDetector
 * Method:    getSerialisedGPUs
 * Signature: ()Ljava/lang/String;
 */
JNIEXPORT jstring JNICALL Java_dev_kostromdan_mods_crash_1assistant_app_utils_gpu_DirectXGPUDetector_getSerialisedGPUs
  (JNIEnv *env, jclass) {
    // Create a buffer to capture all output
    char buffer[4096] = {0};
    
    // Get the GPU detection output
    DetectGPUs(buffer, sizeof(buffer));
    
    // Convert C string to Java String and return it
    return env->NewStringUTF(buffer);
}

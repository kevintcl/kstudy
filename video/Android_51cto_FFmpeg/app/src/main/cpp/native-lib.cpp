#include <jni.h>
#include <string>

#include <android/log.h>

extern "C" JNIEXPORT jstring JNICALL
Java_com_cto_tcl_android_151cto_1ffmpeg_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    __android_log_print(ANDROID_LOG_FATAL, "kevint", "test log for android");
    return env->NewStringUTF(hello.c_str());
}

#include <jni.h>
#include <string>

#include <android/log.h>

#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "kevint", __VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
}
extern "C" JNIEXPORT jstring JNICALL
Java_com_cto_tcl_android_151cto_1ffmpeg_MainActivity_stringFromJNI(
        JNIEnv* env,
        jobject /* this */) {
    std::string hello = "Hello from C++";

    __android_log_print(ANDROID_LOG_DEBUG, "kevint", "test log for android");
    hello += avcodec_configuration();

    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_com_cto_tcl_android_151cto_1ffmpeg_MainActivity_open(JNIEnv *env, jobject thiz, jstring jurl,
                                                          jobject handle) {

    const char *url = env->GetStringUTFChars(jurl, 0);

    FILE *fp = fopen(url, "rb");
    if (fp) {
        LOGD("File %s open Success!", url);
        fclose(fp);
    } else {
        LOGD("File %s open Failed!", url);
    }
    env->ReleaseStringUTFChars(jurl, url);
}


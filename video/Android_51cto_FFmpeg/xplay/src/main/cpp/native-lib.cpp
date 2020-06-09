//
// Created by kevint on 6/9/20.
//

#include <jni.h>
#include <string>
#include "IDemux.h"
#include "FFDemux.h"

extern "C"
JNIEXPORT jstring JNICALL
Java_com_tcl_xplay_MainActivity_invokeFromJava(JNIEnv *env,
        jobject thiz, jstring url) {
    std::string hello = "Hello from C++";
    IDemux de = new FFDemux();
    de.open("/sdcard/videos/1080.mp4");
    return env->NewStringUTF(hello.c_str());
}


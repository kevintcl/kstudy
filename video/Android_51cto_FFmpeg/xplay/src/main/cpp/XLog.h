//
// Created by kevint on 6/9/20.
//

#ifndef ANDROID_51CTO_FFMPEG_XLOG_H
#define ANDROID_51CTO_FFMPEG_XLOG_H

#include <android/log.h>

class XLog {

};

#define XLOGI(...)  __android_log_print(ANDROID_LOG_INFO, "kevint", __VA_ARGS__)
#define XLOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "kevint", __VA_ARGS__)
#define XLOGW(...)  __android_log_print(ANDROID_LOG_WARRING, "kevint", __VA_ARGS__)
#define XLOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "kevint", __VA_ARGS__)
#endif //ANDROID_51CTO_FFMPEG_XLOG_H

//
// Created by kevint on 6/9/20.
//

#ifndef ANDROID_51CTO_FFMPEG_FFDEMUX_H
#define ANDROID_51CTO_FFMPEG_FFDEMUX_H


#include "IDemux.h"

class FFDemux: public IDemux {

public:
    //open file or stream rmtp http rtsp
    virtual bool open(const char *url) = 0;

    //读取one frame data, 数据由调用者清理
    virtual XData read() = 0;


};


#endif //ANDROID_51CTO_FFMPEG_FFDEMUX_H
//
// Created by kevint on 2020/6/7.
//

#include <jni.h>
#include <string>
#include <SLES/OpenSLES.h>
#include <SLES/OpenSLES_Android.h>
#include <android/log.h>

#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "kevint", __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "kevint", __VA_ARGS__)


static SLObjectItf engineSL = nullptr;

SLEngineItf createSL() {
    SLresult re;
    re = slCreateEngine(&engineSL, 0,
                        0, 0,
                        0, 0);
    if (re != SL_RESULT_SUCCESS) {
        return nullptr;
    }

    re = (*engineSL)->Realize(engineSL, SL_BOOLEAN_FALSE);

    if (re != SL_RESULT_SUCCESS) {
        return nullptr;
    }
    SLEngineItf engineItf;
    re = (*engineSL)->GetInterface(engineSL, SL_IID_ENGINE, &engineItf);
    if (re != SL_RESULT_SUCCESS) {
        return nullptr;
    }

    return engineItf;

}

void pcmCall(SLAndroidSimpleBufferQueueItf bf, void *contex);

extern "C"
JNIEXPORT void JNICALL
Java_com_hecate_testopensl_MainActivity_opensl(JNIEnv *env, jobject thiz, jstring jurl) {

    const char *url = env->GetStringUTFChars(jurl, 0);
    //1.创建引擎
    SLEngineItf eng = createSL();

    if (eng) {
        LOGD("create success!");
    } else {
        LOGE("create Failed!");
        return;
    }

    SLresult re = 0;
    //2.创建混音器
    SLObjectItf mix = nullptr;
    re = (*eng)->CreateOutputMix(eng, &mix, 0, nullptr, nullptr);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("CreateOutputMix Failed!");
        return;
    }
    re = (*mix)->Realize(mix, SL_BOOLEAN_FALSE);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("(*mix)->Realize Failed!");
        return;
    }

    SLDataLocator_OutputMix outputMix = {SL_DATALOCATOR_OUTPUTMIX, mix};
    SLDataSink audioSink = {&outputMix, nullptr};

    //3.配置音频信息
    //缓冲队列
    SLDataLocator_AndroidSimpleBufferQueue queue = {SL_DATALOCATOR_ANDROIDSIMPLEBUFFERQUEUE, 10};
    //音频格式
    SLDataFormat_PCM pcm = {
            SL_DATAFORMAT_PCM,
            2,
            SL_SAMPLINGRATE_44_1,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_PCMSAMPLEFORMAT_FIXED_16,
            SL_SPEAKER_FRONT_LEFT | SL_SPEAKER_FRONT_RIGHT,
            SL_BYTEORDER_LITTLEENDIAN
    };
    SLDataSource ds = {&queue, &pcm};

    //4.创建播放器
    SLObjectItf player = nullptr;
    const SLInterfaceID ids[] = {SL_IID_BUFFERQUEUE};
    const SLboolean req[] = {SL_BOOLEAN_TRUE};
    re = (*eng)->CreateAudioPlayer(eng,
                                   &player,
                                   &ds,
                                   &audioSink,
                                   sizeof(ids) / sizeof(SLInterfaceID),
                                   ids,
                                   req);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("CreateAudioPlayer Failed!");
        return;
    } else {
        LOGD("CreateAudioPlayer SUCCESS!");
    }
    //初始化
    (*player)->Realize(player, SL_BOOLEAN_FALSE);

    //获取player接口
    SLPlayItf iPlayer = nullptr;
    re = (*player)->GetInterface(player, SL_IID_PLAY, &iPlayer);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("(*player)->GetInterface Failed!");
        return;
    } else {
        LOGD("(*player)->GetInterface SUCCESS!");
    }

    SLAndroidSimpleBufferQueueItf pcmQue = nullptr;

    re = (*player)->GetInterface(player, SL_IID_BUFFERQUEUE, &pcmQue);
    if (re != SL_RESULT_SUCCESS) {
        LOGE("(*player)->GetInterface SL_IID_BUFFERQUEUE Failed!");
        return;
    } else {
        LOGD("(*player)->GetInterface SL_IID_BUFFERQUEUE SUCCESS!");
    }

    //设置回调函数，播放队列空调用
    (*pcmQue)->RegisterCallback(pcmQue, pcmCall, nullptr);

    //设置为播放状态
    (*iPlayer)->SetPlayState(iPlayer, SL_PLAYSTATE_PLAYING);
    //启动队列回调
    (*pcmQue)->Enqueue(pcmQue, "", 1);

    env->ReleaseStringUTFChars(jurl, url);
}

void pcmCall(SLAndroidSimpleBufferQueueItf bf, void *contex) {
    LOGD("pcmCall");
    static FILE *fp = nullptr;
    if (!fp) {          // "/sdcard/videos/test.pcm";
        fp = fopen("/sdcard/videos/test.pcm", "rb");
    }
    if (!fp) {
        LOGE("error======open file=/sdcard/videos/test.pcm");
        return;
    }

    static char *buf = nullptr;
    if (!buf) {
        buf = new char[1024 * 1024];
    }
    if (feof(fp) == 0) {//表示没到结尾
        int len = fread(buf, 1, 1024, fp);
        if (len > 0) {
            (*bf)->Enqueue(bf, buf, len);
        }
    }
    LOGE("END==============PCM!");
}

//
// Created by kevint on 2020/5/31.
//
#include <jni.h>
#include <string>

#include <android/log.h>


#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "kevint", __VA_ARGS__)
#define LOGE(...)  __android_log_print(ANDROID_LOG_ERROR, "kevint", __VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/jni.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

void open(char *url);

extern "C"
JNIEXPORT jboolean JNICALL
Java_com_cto_tcl_android_151cto_1ffmpeg_c4decapsulation_Decapsulation_open(JNIEnv *env,
                                                                           jclass clazz,
                                                                           jstring url,
                                                                           jobject handle) {
    open("");
    return true;
}

extern "C"
JNIEXPORT
jint JNI_OnLoad(JavaVM *vm, void *res) {
    av_jni_set_java_vm(vm, 0);
    LOGD("=========JNI_OnLoad=====");
    return JNI_VERSION_1_4;
}

static double r2d(AVRational r) {
    return r.num == 0 || r.den == 0 ? 0 : (double) r.num / (double) r.den;
}

//当前时间戳
long long getNowMs() {
    struct timeval tv;
    gettimeofday(&tv, NULL);

    int sec = tv.tv_sec % 360000; //360000 100个小时
    long long t = sec * 1000 + tv.tv_usec / 1000;
    return t;
}

void open(char *url) {
    //1.初始化解封装
    av_register_all();



    //2.初始化网络
    avformat_network_init();

    //解码器注册
    avcodec_register_all();
    //3.打开文件
    AVFormatContext *ic = NULL;
//    char path[] = "/sdcard/videos/0a886809db8a91502eac881d445ba75b.mp4";//1080.mp4
    char path[] = "/sdcard/videos/1080.mp4";//1080.mp4
    int re = avformat_open_input(&ic, path, 0, 0);


    if (re) {
        LOGD("avformat_open_input failed %s", av_err2str(re));
        return;
    }

    LOGD("avformat_open_input success %s ", path);

    //4.获取流信息
    re = avformat_find_stream_info(ic, 0);
    if (!re) {
        LOGD("avformat_find_stream_info failed");
    }
    LOGD("duration=%lld, nb_streams=%d", ic->duration, ic->nb_streams);


    int fps = 0;
    int videoStream = 0;
    int audioSteam = 1;
    for (int i = 0; i < ic->nb_streams; i++) {
        AVStream *as = ic->streams[i];
        if (as->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            LOGD("视频数据:");
            videoStream = i;
            fps = r2d(as->avg_frame_rate);

            // AVPixelFormat
            //0 => AV_PIX_FMT_YUV420P
            //fps=25, width=544 height=960 codeid=28 pix_format=0
            LOGD("fps=%d, width=%d height=%d codeid=%d pix_format=%d", fps,
                 as->codecpar->width, as->codecpar->height, as->codecpar->codec_id,
                 as->codecpar->format);

        } else if (as->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            LOGD("音频数据");
            audioSteam = i;
            //AVSampleFormat
            //8 =>AV_SAMPLE_FMT_FLTP
            //sample_rate=44100 channels=2 sample_format=8
            LOGD("sample_rate=%d channels=%d sample_format=%d",
                 as->codecpar->sample_rate,
                 as->codecpar->channels,
                 as->codecpar->format);
        }
    }

    //获取音频流信息
    audioSteam = av_find_best_stream(ic, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    LOGD("av_find_best_stream audioSteam=%d", audioSteam);


    //软件解码器
//    AVCodec *codec = avcodec_find_decoder(ic->streams[videoStream]->codecpar->codec_id);

    //硬解码
    AVCodec *codec = avcodec_find_decoder_by_name("h264_mediacodec");

    if (!codec) {
        LOGD("avcodec_find_decode video error!");
        return;
    }

    //解码器初始化
    AVCodecContext *vc = avcodec_alloc_context3(codec);
    avcodec_parameters_to_context(vc, ic->streams[videoStream]->codecpar);
    vc->thread_count = 1;

    //打开视频解码器
    re = avcodec_open2(vc, 0, 0);
    if (re) {
        LOGD("avcodec_open2 video error!");
        return;
    }

    AVCodec *acodec = avcodec_find_decoder(ic->streams[audioSteam]->codecpar->codec_id);

    if (!acodec) {
        LOGD("avcodec_find_decoder audio error!");
        return;
    }

    //打开音频解码器
    AVCodecContext *ac = avcodec_alloc_context3(acodec);
    avcodec_parameters_to_context(ac, ic->streams[audioSteam]->codecpar);
    ac->thread_count = 1;
    //#####################################################3
    //打开音频解码器
    re = avcodec_open2(ac, 0, 0);
    if (re) {
        LOGD("avcodec_open2 audio error!");
        return;
    }

    //AVFormatContext *s, AVPacket *pkt

    //读取帧数据
    AVPacket *pkt = av_packet_alloc();

    long long start = getNowMs();
    int frame_count = 0;

    AVFrame *frame = av_frame_alloc();
    int outWidth = 1080;
    int outHeight = 720;


    //初始化像素格式转换上下文
    SwsContext *vctx = NULL;
    char *rgbBuf = new char[1920 * 1080 * 4];

    char *pcm = new char[48000 * 4 * 2];

    //音频重采样上下文初始化
    SwrContext *actx = swr_alloc();
    actx = swr_alloc_set_opts(actx,
                              av_get_default_channel_layout(2),
                              AV_SAMPLE_FMT_S16, //非平面格式
                              ac->sample_rate,
                              av_get_default_channel_layout(ac->channels),
                              ac->sample_fmt,
                              ac->sample_rate,
                              0, 0);
    re = swr_init(actx);

    if (re != 0) {
        LOGE("swr_init Failed!");
        return;
    } else {
        LOGE("swr_init Success!");
    }

    for (;;) {
        //超过3s
        if (getNowMs() - start >= 1000) {
            LOGD("now decode fps is %d", frame_count / 1);
            start = getNowMs();
            frame_count = 0;
        }

        int re = av_read_frame(ic, pkt);
        if (re != 0) {
            LOGD("READ TO END!");
//            int pos = 20 * r2d(ic->streams[videoStream]->time_base); //20s
//            av_seek_frame(ic, videoStream, pos, AVSEEK_FLAG_BACKWARD | AVSEEK_FLAG_FRAME);
            break;
        }


        AVCodecContext *cc = vc;

        if (pkt->stream_index != videoStream) {
            cc = ac;
        }
        //发送到线程解码，pkt 会复制一份
        re = avcodec_send_packet(cc, pkt);
        av_packet_unref(pkt);
        if (re != 0) {
            LOGD("avcodec_send_packet failed!");
            continue;
        }


        for (;;) {

            re = avcodec_receive_frame(cc, frame);

            if (re != 0) {
//                LOGD("avcodec_receive_frame failed!");
                break;
            }
            //如果是视频
            if (cc == vc) {
                frame_count++;

                vctx = sws_getCachedContext(
                        vctx,
                        frame->width,
                        frame->height,
                        (AVPixelFormat) frame->format,
                        outWidth,
                        outHeight,
                        AV_PIX_FMT_RGBA,
                        SWS_FAST_BILINEAR,
                        0,
                        0,
                        0
                );
                if (!vctx) {
                    LOGD("sws_getCachedContext failed");

                } else {
                    uint8_t *data[AV_NUM_DATA_POINTERS] = {0};
                    data[0] = (uint8_t *) rgbBuf;

                    int lines[AV_NUM_DATA_POINTERS] = {0};
                    lines[0] = outWidth * 4;//RGBA
                    int h = sws_scale(vctx,
                                      frame->data,
                                      frame->linesize,
                                      0,
                                      frame->height,
                                      data,
                                      lines);
                    LOGD("sws_scale==%d", h);


                }
            } else {//音频
                //音频重采样
                uint8_t *out[2] = {0};
                out[0] = (uint8_t *) pcm;
                int len = swr_convert(actx,
                                      out,
                                      frame->nb_samples,
                                      (const uint8_t **) frame->data,
                                      frame->nb_samples);
                LOGD("swr_convert==%d", len);
            }
        }


    }

    delete rgbBuf;
    delete pcm;
    avformat_close_input(&ic);

}

void use() {
    /** 音频
    struct SwrContext *swr_alloc(void);

    struct SwrContext *swr_alloc_set_opts(
            struct SwrContext *s,
            int64_t out_ch_layout,
            enum AVSampleFormat out_sample_fmt,
            int out_sample_rate,
            int64_t in_ch_layout,
            enum AVSampleFormat in_sample_fmt,
            int in_sample_rate,
            int log_offset,
            void *log_ctx);
    int swr_init(struct SwrContext *s);
    void swr_free(struct SwrContext **s);


    int swr_convert(struct SwrContext *s,
                    uint8_t **out,
                    int out_count,
                    const uint8_t **in,
                    int in_count //nbsamples 单通道样本数量，双通道要 x2
                    );

    **/

}



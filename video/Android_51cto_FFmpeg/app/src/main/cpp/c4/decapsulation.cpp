//
// Created by kevint on 2020/5/31.
//
#include <jni.h>
#include <string>

#include <android/log.h>

#define LOGD(...)  __android_log_print(ANDROID_LOG_DEBUG, "kevint", __VA_ARGS__)
extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libavcodec/jni.h>
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
    char path[] = "/sdcard/videos/0a886809db8a91502eac881d445ba75b.mp4";//1080.mp4
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

    for (;;) {
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
        AVFrame *frame = av_frame_alloc();

        for (;;) {
            //超过3s
            if (getNowMs() - start >= 1000) {
                LOGD("now decode fps is %d", frame_count / 1);
                start = getNowMs();
                frame_count = 0;
            }
            re = avcodec_receive_frame(cc, frame);

            if (re != 0) {
//                LOGD("avcodec_receive_frame failed!");
                break;
            }
            //如果是视频
            if (cc == vc ) {
                frame_count++;
            }
//            LOGD("avcodec_receive_frame %lld", frame->pts);
        }


    }



//    av_read_frame()

    avformat_close_input(&ic);

}



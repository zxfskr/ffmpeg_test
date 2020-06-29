// #include "stdafx.h"
 
#include <stdio.h>  
 
#define __STDC_CONSTANT_MACROS  
 
#ifdef _WIN32  
//Windows  
extern "C"
{
#include "libavcodec/avcodec.h"  
#include "libavformat/avformat.h"  
#include "libswscale/swscale.h"  
#include "SDL2/SDL.h"  
};
#else  
//Linux...  
#ifdef __cplusplus  
extern "C"
{
#endif  

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
#include <libavutil/avutil.h>
#include <libavdevice/avdevice.h>
#include <libswresample/swresample.h>
// #include <SDL2/SDL.h>  

#ifdef __cplusplus  
};
#endif  
#endif  

static int encode_num = 0;


SwrContext * init_swr(){
    int ret = -1;
    //channel, number/layout   
    SwrContext *swr_ctx = swr_alloc_set_opts(
        NULL,
        AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_S16,
        44100,
        AV_CH_LAYOUT_STEREO,
        AV_SAMPLE_FMT_S16,
        44100,
        0,NULL);

    if (!swr_ctx){
        printf("error swr\n");
        return NULL;
    }

    ret = swr_init(swr_ctx);
    if (ret < 0){
        printf("err swr init");
        return NULL;
    }
    return swr_ctx;
}


AVCodecContext *open_coder(){
    int ret = -1;
    // 打开编码器
    AVCodec *codec = avcodec_find_encoder_by_name("libfdk_aac");
    //创建上下文
    AVCodecContext *codec_ctx = avcodec_alloc_context3(codec);
    codec_ctx->sample_fmt = AV_SAMPLE_FMT_S16; // libfdk_aac 只能处理s16le格式
    codec_ctx->channel_layout = AV_CH_LAYOUT_STEREO;
    codec_ctx->sample_rate = 44100;
    codec_ctx->channels = 2;
    codec_ctx->bit_rate = 0; //aac_lc: 128k; aac_he: 64k; aac_he_v2: 32k
    codec_ctx->profile = FF_PROFILE_AAC_HE_V2; //设置此项需要先将bit rate设置为0

    ret = avcodec_open2(codec_ctx, codec, NULL);
    if (ret < 0){
        printf("error open codec");
        return NULL;
    }

    return codec_ctx;
}

void encode(AVCodecContext *codec_ctx, AVFrame *frame, AVPacket *newpkt, FILE *ofp){
    int ret = -1;
    ret = avcodec_send_frame(codec_ctx, frame);

    //如果ret>=0说明数据设置成功
    while(ret >= 0){
        //获取编码后的音频数据,如果成功，需要重复获取，直到失败为止
        ret = avcodec_receive_packet(codec_ctx, newpkt);
        
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }else if( ret < 0){
            printf("Error, encoding audio frame\n");
            exit(-1);
        }
        encode_num++;

        av_log(NULL, AV_LOG_INFO, "pkt size: %d(%p), encode_count: %d\n",
                newpkt->size, newpkt->data, encode_num);
        
        //write file
        fwrite(newpkt->data, 1, newpkt->size, ofp);
        fflush(ofp);
    }
}

void record_audio(){
	int ret = 0;
    int count = 0;
    char *ofilename = "./video/out.aac";
    FILE *ofp = NULL;

    AVFormatContext *fmt_ctx = NULL;
    char *devicename = "hw:0";
	AVDictionary *options = NULL;
	char errors[1024] = {0,};
    AVPacket pkt;

    SwrContext *swr_ctx = NULL;

    // register all
    avdevice_register_all();

    //set log level
    av_log_set_level(AV_LOG_DEBUG);

    //get format
    AVInputFormat *iformat = av_find_input_format("alsa");
    
    //open device alloc ctx
    ret = avformat_open_input(&fmt_ctx, devicename, iformat, &options);
	if (ret < 0){
		av_strerror(ret, errors, 1024);
		fprintf(stderr, "Failed to open audio device, [%d]%s\n", ret, errors);
		return;
	}

    //create file 
    ofp = fopen(ofilename, "wb+");
    if (ofp == NULL){
        av_log(NULL, AV_LOG_ERROR, "failed to open %s\n", ofilename);
        return;
    }

    AVCodecContext *codec_ctx = open_coder();

    AVFrame *frame = av_frame_alloc();

    frame->nb_samples = 512; //单通道样本数据数量
    frame->format = AV_SAMPLE_FMT_S16; // 样本数据
    frame->channel_layout = AV_CH_LAYOUT_STEREO;
    ret = av_frame_get_buffer(frame, 0); // 512*16/8*2
    if (!frame || !frame->buf[0]){
        return;
    }

    AVPacket *newpkt = av_packet_alloc();
    if (!newpkt){
        return;
    }

    swr_ctx = init_swr();

    uint8_t **src_data = NULL;
    int src_linesize = 0;

    uint8_t **dst_data = NULL;
    int dst_linesize = 0;

    // 2048*8/16/2 16位深， 双通道
    av_samples_alloc_array_and_samples(
        &src_data, //输出缓冲区地址
        &src_linesize, // 缓冲区大小
        2, //通道数
        512, // 单通道采样数据个数
        AV_SAMPLE_FMT_S16, //每个采样数据   7格式
        0);

    // 2048*8/16/2 16位深， 双通道
    av_samples_alloc_array_and_samples(
        &dst_data, //输出缓冲区地址
        &dst_linesize, // 缓冲区大小
        2, //通道数
        512, // 单通道采样数据个数
        AV_SAMPLE_FMT_S16, //每个采样数据   7格式
        0);
    
    av_init_packet(&pkt);
    while (1)
    {
        /* code */
        //read data from device
        ret = av_read_frame(fmt_ctx, &pkt);
        if (ret == 0){
            av_log(NULL, AV_LOG_INFO, "pkt size: %d(%p), count =%d\n",
                pkt.size, pkt.data, count);
        }

        memcpy((void *)src_data[0], (void *)pkt.data, pkt.size);

        swr_convert(
            swr_ctx,
            dst_data,
            512,
            (const uint8_t **)src_data,
            512);

        memcpy((void *)frame->data[0], dst_data[0], dst_linesize);

        encode(codec_ctx, frame, newpkt, ofp);
        //write file
        // fwrite(pkt.data, pkt.size, 1, ofp);
        // fwrite(dst_data[0], dst_linesize, 1, ofp);
        // fflush(ofp);
        
        //release pkt  
        av_packet_unref(&pkt);

        count++;
        if (count > 500){
            break; 
        }
    }

    encode(codec_ctx, NULL, newpkt, ofp);

    fclose(ofp);

    // 释放缓冲区
    if (src_data){
        av_freep(&src_data[0]);
    }

    av_freep(&src_data);

    if (dst_data){
        av_freep(&dst_data[0]);
    }

    av_freep(&dst_data);

    // 释放重采样上下文
    swr_free(&swr_ctx);

    //close device release ctx
    avformat_close_input(&fmt_ctx);

    av_log(NULL, AV_LOG_DEBUG, "finish\n");
    return;
}

// int main(int argc, char **argv){
// 	record_audio();
// 	return 0;
// }
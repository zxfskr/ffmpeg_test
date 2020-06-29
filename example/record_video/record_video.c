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
#include <libyuv.h>

// #include <SDL2/SDL.h>  
#ifdef __cplusplus  
};
#endif  
#endif  

AVFormatContext *open_dev(){
    AVFormatContext *fmt_ctx = NULL;
    char errors[1024] = {0,};
    int ret = -1;
    char *devicename = "/dev/video0";
    AVDictionary *options = NULL;
    //get format
    AVInputFormat *iformat = av_find_input_format("v4l2");


    // 设置不会其作用，以设备输出为准
    // av_dict_set(&options, "video_size", "640x480", 0);
    // av_dict_set(&options, "framerate", "25", 0);
    // av_dict_set(&options, "pixel_format", "yuyv422", 0);
    
    //open device alloc ctx
    ret = avformat_open_input(&fmt_ctx, devicename, iformat, &options);
	if (ret < 0){
		av_strerror(ret, errors, 1024);
		fprintf(stderr, "Failed to open audio device, [%d]%s\n", ret, errors);
		return NULL;
	}
    // int width = fmt_ctx->streams[0]->codecpar->width;
    // printf("输入设备宽:%d\n", width);
    return fmt_ctx;
}

void open_encoder(int width, int height, AVCodecContext **enc_ctx){
    AVCodec *codec = NULL;
    AVCodecContext *tmp_ctx = NULL;
    int ret = -1;
    codec = avcodec_find_encoder_by_name("libx264");
    if (!codec){
        printf("cant find libx264 !\n");
        exit(-1);
    }

    tmp_ctx = avcodec_alloc_context3(codec);
    if (!tmp_ctx){
        printf("alloc libx264 ctx failed!\n");
        exit(-1);
    }
    //sps pps
    tmp_ctx->profile = FF_PROFILE_H264_HIGH_444;
    tmp_ctx->level = 50; // 5.0
    tmp_ctx->width = width;
    tmp_ctx->height = height;

    tmp_ctx->gop_size = 250;
    tmp_ctx->keyint_min = 25;

    tmp_ctx->max_b_frames = 3;
    tmp_ctx->has_b_frames = 1;

    tmp_ctx->refs = 3;
    tmp_ctx->pix_fmt = AV_PIX_FMT_YUV420P;
    tmp_ctx->bit_rate = 600000;

    tmp_ctx->framerate = (AVRational){25, 1}; 
    tmp_ctx->time_base = (AVRational){1, 25};

    *enc_ctx = tmp_ctx;

    ret = avcodec_open2(tmp_ctx, codec, NULL);
    if (ret < 0){
        printf("cant open codec: %s\n", av_err2str(ret));
        exit(-1);
    }

}

AVFrame *create_frame(int width, int height){
    int ret = -1;
    AVFrame *frame = av_frame_alloc();
    if (!frame){
        printf("error to alloc frame");
        goto __ERROR;
    }

    frame->width = width;
    frame->height = height;
    frame->format = AV_PIX_FMT_YUV420P;

    ret = av_frame_get_buffer(frame, 32); //按32位对齐
    if (ret < 0){
        printf("error to alloc frame buf\n");
        goto __ERROR;
    }
    return frame;

__ERROR:
    if (frame){
        av_frame_free(&frame);
    }
    return NULL;
    // exit(-1);
}

void encode(AVCodecContext *enc_ctx, AVFrame *frame, AVPacket *newpkt, FILE *ofp){
    int ret = -1;

    if (frame)
        printf("send frame to encoder , pts=%lld\n", frame->pts);
    ret = avcodec_send_frame(enc_ctx, frame);
    if (ret < 0){
        printf("error, failed to send frame to codec\n");
        exit(-1);
    }

    while (1)
    {
        /* code */
        ret = avcodec_receive_packet(enc_ctx, newpkt);
        if(ret == AVERROR(EAGAIN) || ret == AVERROR_EOF){
            return;
        }else if( ret < 0){
            printf("Error, encoding audio frame\n");
            exit(-1);
        }
        
        fwrite(newpkt->data, newpkt->size, 1, ofp);
        fflush(ofp);
    }
    
}

void record_video(){
	int ret = 0;
    int count = 0;
    char *ofilename = "./video/out.h264";
    FILE *ofp = NULL;
    int base = 0; 

    int src_w, src_h;

    uint8_t *i420_image = NULL;

    AVFormatContext *fmt_ctx = NULL;
    AVCodecContext *enc_ctx = NULL;
    AVPacket pkt;

    // register all
    avdevice_register_all();

    //set log level
    av_log_set_level(AV_LOG_DEBUG);

    //create file 
    ofp = fopen(ofilename, "wb+");
    if (ofp == NULL){
        av_log(NULL, AV_LOG_ERROR, "failed to open %s\n", ofilename);
        return;
    }

    fmt_ctx = open_dev();
    src_w = fmt_ctx->streams[0]->codecpar->width;
    src_h = fmt_ctx->streams[0]->codecpar->height;

    open_encoder(src_w, src_h, &enc_ctx);

    // 创建avframe 
    AVFrame *frame = create_frame(src_w, src_h);

    AVPacket *newpkt = av_packet_alloc();

    i420_image = (uint8_t *)malloc(src_w * src_h * 1.5);
    if (!i420_image){
        printf("i420 临时图片空间分配失败\n");
        exit(-1);
    }

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

        // fwrite(pkt.data, pkt.size, 1, ofp);
        // fflush(ofp);

        int y_size = src_w*src_h;
        int u_size = y_size/4;
        uint8_t *i420_image_y = i420_image;
        uint8_t *i420_image_u = i420_image + y_size;
        uint8_t *i420_image_v = i420_image_u + u_size;

        printf("pkt_size: %d, y_size: %d\n", pkt.size, y_size);

        printf("func piont: %p\n", YUY2ToI420);
        YUY2ToI420(
            pkt.data,
            src_w*2,
            i420_image_y,
            src_w,
            i420_image_u,
            src_w/2,
            i420_image_v,
            src_w/2,
            src_w,
            src_h);

        memcpy(frame->data[0], i420_image_y, y_size);
        memcpy(frame->data[1], i420_image_u, u_size);
        memcpy(frame->data[2], i420_image_v, u_size);
        
        frame->pts = pkt.pts;
        encode(enc_ctx, frame, newpkt, ofp);
        // fwrite(i420_image, y_size*1.5, 1, ofp);
        // fflush(ofp);
        
        //release pkt  
        av_packet_unref(&pkt);

        count++;
        if (count > 100){
            break; 
        }
    }

    encode(enc_ctx, NULL, newpkt, ofp);

    fclose(ofp);

    //close device release ctx
    avformat_close_input(&fmt_ctx);

    av_log(NULL, AV_LOG_DEBUG, "finish\n");
    return;
}

int main(int argc, char **argv){
	record_video();
	return 0;
}
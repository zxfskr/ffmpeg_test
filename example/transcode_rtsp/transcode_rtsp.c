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
#include <libavcodec/avcodec.h>  
#include <libavformat/avformat.h>  
#include <libavutil/avutil.h>
#include <SDL2/SDL.h>  
#ifdef __cplusplus  
};
#endif  
#endif  

static void flush_decode(AVCodecContext *dec_ctx, AVFrame *frame, FILE *f)
{
    int ret = 1;
    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }


        printf("saving frame %3d\n", dec_ctx->frame_number);
        // fflush(stdout);

        fwrite(frame->data[0], frame->width, frame->height, f);
        fwrite(frame->data[1], frame->width/2, frame->height/2, f);
        fwrite(frame->data[2], frame->width/2, frame->height/2, f);
        
        printf("w: %d, h: %d\n", frame->width, frame->height);
        // exit(1);
    }
}

static void decode(AVCodecContext *dec_ctx, AVFrame *frame, AVPacket *pkt,
                FILE *f)
{
    // char buf[1024];
    int ret;

    ret = avcodec_send_packet(dec_ctx, pkt);
    if (ret < 0) {
        fprintf(stderr, "Error sending a pkt for decoding\n");
        exit(1);
    }

    while (ret >= 0) {
        ret = avcodec_receive_frame(dec_ctx, frame);
        if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
            return;
        else if (ret < 0) {
            fprintf(stderr, "Error during decoding\n");
            exit(1);
        }


        printf("saving frame %3d\n", dec_ctx->frame_number);
        // fflush(stdout);

        fwrite(frame->data[0], frame->width, frame->height, f);
        fwrite(frame->data[1], frame->width/2, frame->height/2, f);
        fwrite(frame->data[2], frame->width/2, frame->height/2, f);
        
        printf("w: %d, h: %d\n", frame->width, frame->height);
        // exit(1);
        /* the picture is allocated by the decoder. no need to
           free it */
        
        // snprintf(buf, sizeof(buf), "%s-%d", filename, dec_ctx->frame_number);
        // pgm_save(frame->data[0], frame->linesize[0],
        //          frame->width, frame->height, buf);
    }

}

int main(){
    AVFormatContext	*pFormatCtx = NULL;
    int i, videoindex,audioindex;
    AVPacket *pkt = NULL;
    AVDictionary *options = NULL;
 
    //下面是公共的RTSP测试地址
    char *filepath = "rtsp://192.168.1.199/zxf-test";
    avformat_network_init();

    av_dict_set(&options, "rtsp_transport", "tcp", 0);                //采用tcp传输
    av_dict_set(&options, "stimeot", "2000000", 0);

    if (avformat_open_input(&pFormatCtx, filepath, NULL, &options) != 0)////打开网络流或文件流
    {
        printf("Couldn't open input stream.\n");
        return -1;
    }

    if (avformat_find_stream_info(pFormatCtx, NULL) < 0)
    {
        printf("Couldn't find stream information.\n");
        return -1;
    }

    videoindex = -1;
    audioindex = -1;
    for (i = 0; i < pFormatCtx->nb_streams; i++) {
        if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            if (videoindex != -1) {
                continue;
            }
            videoindex = i;
        }
        else if (pFormatCtx->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
            if (audioindex != -1) {
                continue;
            }
            audioindex = i;
        }
        else {
            break;
        }
    }
        
    if (videoindex == -1)
    {
        printf("Didn't find a video stream.\n");
        return -1;
    }

    pkt = av_packet_alloc();
    if (!pkt)
    {
        exit(1);
    }

    FILE *fpSave;
    FILE *fpAAcSave;
    if ((fpSave = fopen("out.yuv", "wb")) == NULL) //h264保存的文件名
        return 0;
    if ((fpAAcSave = fopen("out.aac", "wb")) == NULL) //h264保存的文件名
        return 0;

    char *padts = (char *)malloc(sizeof(char) * 7);
    int profile = 2;    //AAC LC  
    int freqIdx = 4;  //44.1KHz  
    int chanCfg = 2;
    padts[0] = (char)0xFF; // 11111111     = syncword  
    padts[1] = (char)0xF1; // 1111 1 00 1  = syncword MPEG-2 Layer CRC  
    padts[2] = (char)(((profile - 1) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    padts[6] = (char)0xFC;

    AVCodec *codec = NULL;
    codec = avcodec_find_decoder(pFormatCtx->streams[videoindex]->codecpar->codec_id);

    AVCodecContext *pCodecCtx = avcodec_alloc_context3(codec);
    if (!pCodecCtx) {
        fprintf(stderr, "Could not allocate video codec context\n");
        exit(1);
    }

    avcodec_parameters_to_context(pCodecCtx, pFormatCtx->streams[videoindex]->codecpar);

    printf("pCodecCtx width: %d, hight: %d\n", pCodecCtx->width, pCodecCtx->height);
    avcodec_open2(pCodecCtx, codec, NULL);

    AVFrame *frame = av_frame_alloc();
    if (!frame) {
        fprintf(stderr, "Could not allocate video frame\n");
        exit(1);
    }

    i = 0;
    for (;i < 300;)
    {
        //------------------------------
        if (av_read_frame(pFormatCtx, pkt) >= 0)
        {
            if (pkt->stream_index == videoindex)
            {
                i++;
                decode(pCodecCtx, frame, pkt, fpSave);
            }
            else if (pkt->stream_index == audioindex)// RTMP需要增加AAC的ADTS头（RTSP不用）
            {
                // printf("write audio frame %d\n", i);
                padts[3] = (char)(((chanCfg & 3) << 6) + ((7 + pkt->size) >> 11));// 增加ADTS头
                padts[4] = (char)(((7 + pkt->size) & 0x7FF) >> 3);
                padts[5] = (char)((((7 + pkt->size) & 7) << 5) + 0x1F);
                fwrite(padts, 7, 1, fpAAcSave);
                fwrite(pkt->data, 1, pkt->size, fpAAcSave);//写数据到文件中
            }
        }
    }

    flush_decode(pCodecCtx, frame, fpSave);

    uint8_t endcode[] = { 0, 0, 1, 0xb7 };
    fwrite(endcode, 1, sizeof(endcode), fpSave);

    av_frame_free(&frame);
    av_packet_free(&pkt);
    avformat_close_input(&pFormatCtx);
    return 0;
}
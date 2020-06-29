// #include "pushstream.h"

#include <libavformat/avformat.h>
#include <libavutil/log.h>

int main(int argc, char *argv[])
{
    char *filename = "./video/input.mp4";
    int ret = -1;
    AVFormatContext *fmt_ctx = NULL;
    char *src = NULL;
    char *dst = NULL;
    int audio_index;
    int len;

    AVPacket pkt;

    av_log_set_level(AV_LOG_INFO);
    // av_register_all();

    if (argc < 3){
        av_log(NULL, AV_LOG_ERROR, "the count of param should be three!\n");
        return -1;
    }

    src = argv[1];
    dst = argv[2];

    if (src == NULL || dst == NULL) {
        av_log(NULL, AV_LOG_ERROR, "src dst is NULL\n");
    }

    ret = avformat_open_input(&fmt_ctx, filename, NULL, NULL);
    if (ret < 0){
        av_log(NULL, AV_LOG_ERROR, "cant open file: %s\n", av_err2str(ret));
        return -1;
    }

    printf("%s\n", dst);
    FILE *dst_fp = fopen(dst, "wb");
    if (!dst_fp){
        av_log(NULL, AV_LOG_ERROR, "cant open dst file\n");
        avformat_close_input(&fmt_ctx);
        return -1;
    }

    av_dump_format(fmt_ctx, 0, filename, 0);

    //2. get stream
    ret = av_find_best_stream(fmt_ctx, AVMEDIA_TYPE_AUDIO, -1, -1, NULL, 0);
    if (ret < 0){
        av_log(NULL, AV_LOG_ERROR, "cant find best stream\n");
        avformat_close_input(&fmt_ctx);
        fclose(dst_fp);
        return -1;
    }

    audio_index = ret;

    av_init_packet(&pkt);

    while (av_read_frame(fmt_ctx, &pkt) >= 0)
    {
        /* code */
        if (pkt.stream_index == audio_index){
            len = fwrite(pkt.data, 1, pkt.size, dst_fp);
            if (len != pkt.size){
                av_log(NULL, AV_LOG_WARNING, "fwirte len not equal size of pkt \n");
            }
        }

        av_packet_unref(&pkt);

    }
    
    avformat_close_input(&fmt_ctx);
    if (dst_fp)
        fclose(dst_fp);
    return 0;
} 

#ifndef COMMON_MY_LOG_H
#define COMMON_MY_LOG_H
#include <libavutil/log.h>

void my_init_av_log(int level);
void my_info(char *fmt, ...);

#endif
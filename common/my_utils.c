#include "my_log.h"

struct AVDictionary {
    int count;
    AVDictionaryEntry *elems;
};

void my_print_avdict(const AVDictionary *m)
{
    int i = 0;
    
    for (; i < m->count; i++) {
        printf("<avdict key: %s, value: %s\n", m->elems[i].key, m->elems[i].value);
    }
}

void my_init_av_log(int level)
{
    av_log_set_flags(AV_LOG_SKIP_REPEATED);
    av_log_set_level(level);
}

void my_info(char *fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);
    av_log(NULL, AV_LOG_INFO, fmt, vl);
    // int n = 2;
    // while (n > 0)
    // {
    //     n--;
    //     char *tmp = va_arg(vl, char *);
    //     av_log(NULL, AV_LOG_INFO, tmp);
    // }
    va_end(vl);
}
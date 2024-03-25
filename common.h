#ifndef _COMMON_H
#define _COMMON_H

#include <time.h>

#ifdef DEBUG_PRINT
#define DEBUG_PRINT(fmt, ...) \
    do { \
        time_t t = time(NULL); \
        struct tm *tm_info = localtime(&t); \
        printf("%04d-%02d-%02d %02d:%02d:%02d DEBUG ", \
                tm_info->tm_year+1900, tm_info->tm_mon, tm_info->tm_mday, \
                tm_info->tm_hour, tm_info->tm_min, tm_info->tm_sec); \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#ifndef CURRENT_UNIX_TIMESTAMP
#define CURRENT_UNIX_TIMESTAMP 0
#endif


#endif
#ifndef _COMMON_H
#define _COMMON_H

#ifdef DEBUG
#define DEBUG_PRINT(fmt, ...) \
    do { \
        printf("DEBUG "); \
        printf(fmt, ##__VA_ARGS__); \
    } while (0)
#else
#define DEBUG_PRINT(fmt, ...) do {} while(0)
#endif

#ifndef CURRENT_UNIX_TIMESTAMP
#define CURRENT_UNIX_TIMESTAMP 0
#endif


#endif

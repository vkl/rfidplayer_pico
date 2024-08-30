#ifndef CASTS_H
#define CASTS_H

#include <lwip/inet.h>

typedef struct {
    char *name;
    char *hostname;
    char *txtData;
    ip_addr_t ipaddr;
    u16_t port;
} ChromeCastDevice;

typedef struct {
    u8_t size;
    u8_t capacity;
    ChromeCastDevice *chromecastDevice;
} ChromeCastDevices;

void initChromeCastDevices(ChromeCastDevices *devices, u8_t capacity);
int addChromeCastDevice(ChromeCastDevices *devices, const char *name,
        const char *hostname, const char *txtData, ip_addr_t ipaddr, u16_t port);
void freeChromeCastDevices(ChromeCastDevices *devices);
ChromeCastDevice *findChromeCastDevice(ChromeCastDevices *devices, const char *name);

//extern struct ChromeCast casts[2];

//struct ChromeCast* findChromecastByName(struct ChromeCast *casts, int size, const char* name);

#endif // CASTS_H

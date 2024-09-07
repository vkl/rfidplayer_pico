#include <string.h>
#include "casts.h"

ChromeCastDevice *findChromeCastDevice(ChromeCastDevices *devices, const char *fname) {
    printf("fname: %s\n", fname);
    for (u8_t i=0; i < devices->size; i++) {
        if (strstr(devices->chromecastDevice[i].txtData, fname))
            return &devices->chromecastDevice[i];
    }
    return NULL;
}

void initChromeCastDevices(ChromeCastDevices *devices, u8_t capacity) {
    devices->size = 0;
    devices->capacity = capacity;
    devices->chromecastDevice = calloc(capacity, sizeof(ChromeCastDevice));
}

int addChromeCastDevice(ChromeCastDevices *devices, const char *name,
        const char *txtData, const char *hostname, ip_addr_t ipaddr, u16_t port) {
    
    printf("add device: %s\n", name);
    if (devices->size >= devices->capacity) {
        ChromeCastDevice *newDevices = realloc(devices->chromecastDevice,
                (devices->size + 1) * sizeof(ChromeCastDevice));
        if (newDevices == NULL) {
            return -1;
        }
        devices->capacity++;
        devices->chromecastDevice = newDevices;
    }

    devices->chromecastDevice[devices->size].name = strdup(name);
    devices->chromecastDevice[devices->size].hostname = strdup(hostname);
    devices->chromecastDevice[devices->size].txtData = strdup(txtData);
    devices->chromecastDevice[devices->size].ipaddr = ipaddr;
    devices->chromecastDevice[devices->size].port = port;

    devices->size++;
    return 0;
}

void freeChromeCastDevices(ChromeCastDevices *devices) {
    for (u16_t i = 0; i < devices->size; i++) {
        free(devices->chromecastDevice[i].name);
        free(devices->chromecastDevice[i].hostname);
    }
    free(devices->chromecastDevice);
    devices->chromecastDevice = NULL;
    devices->size = 0;
}


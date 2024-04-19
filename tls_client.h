#ifndef _TLS_CLIENT_H
#define _TLS_CLIENT_H

#include "lwip/err.h"

#include "pico/sync.h"

#include "common.h"
#include "cast_message.h"
#include "cast_controllers.h"

#define NOT_CONNECTED        0
#define CONNECTING           1
#define CONNECTED            2
#define READY_TO_SEND        3
#define REQUEST_PENDING      4
#define INITIAL_DATA_PACKAGE 5
#define WAITING_MORE_DATA    6
#define DATA_READY           7
#define CONNECTION_CLOSE     8

// typedef uint8_t (*processingData_fn)(unsigned char *data);
// typedef void (*processingData_fn)(unsigned char *data, void *arg);

struct MessageItem {
    // enum CastMessageType castType;
    struct MessageItem *next;
    unsigned char *msg;
    size_t msgLen;
};

struct connectionState {
    int state;
    struct altcp_pcb *pcb;
    char *recvData;
    struct MessageItem *item;
    int start;
    // processingData_fn processingData;
};

int pollConnection(struct connectionState **pcs);
struct connectionState *doConnect(const char *host, int port); //, processingData_fn processingData);

extern semaphore_t semafore;

#endif

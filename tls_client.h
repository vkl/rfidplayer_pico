#ifndef _TLS_CLIENT_H
#define _TLS_CLIENT_H

#include <lwip/inet.h>
#include <cast_control.h>

#define NOT_CONNECTED        0
#define CONNECTING           1
#define CONNECTED            2
#define READY_TO_SEND        3
#define REQUEST_PENDING      4
#define INITIAL_DATA_PACKAGE 5
#define WAITING_MORE_DATA    6
#define DATA_READY           7
#define CONNECTION_CLOSE     8
#define PREPARE_DATA         9

struct ConnectionState {
    int state;
    struct altcp_pcb *pcb;
    unsigned char *recvData;
    int start;
    struct CastState *castState;
};

int pollConnection(struct ConnectionState **pcs,
        struct MessageQueueItem **msgQueueItem,
        struct MessageItem *msgItem);
struct ConnectionState *doConnect(ip_addr_t ip, const char *hostname, int port);

#endif

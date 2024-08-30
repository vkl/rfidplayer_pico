#ifndef _CAST_CONTROLLERS_H
#define _CAST_CONTROLLERS_H

#include <stdint.h>

#include "tls_client.h"
#include "cast_message.h"
#include "casts.h"

#define MAXFAILEDPINGS 3

struct ChromeCastInfo {
    char *Name;
    char *Value;
};

struct ChromeCast {
    char *Name;
    char *IPAddr;
    int Port;
};

struct CastConnectionState {
    char *IPAddr;
    int Port;
    char *senderId;
    char *receiverId;
    char *appId;
    uint16_t requestId;
    // uint8_t mediaSessionId;
    struct connectionState *cs;
    int8_t pingCount;
    // enum CardEvent cardEvent;
    // struct RfidCard *rfidCard;
    struct MediaStatus mediaStatus;
    // int8_t volume;
};

struct Message {
    char *msg;
    size_t msgLen;
};


void addMessage(struct CastConnectionState *self, enum CastMessageType msgType);
void initCastConnectionState(struct CastConnectionState *self);
void CastConnect(struct CastConnectionState *self, ChromeCastDevices *devices);

extern struct CastConnectionState cast;
//extern semaphore_t semafore;

#endif

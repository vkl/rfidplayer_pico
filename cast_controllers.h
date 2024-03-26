#ifndef _CAST_CONTROLLERS_H
#define _CAST_CONTROLLERS_H

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"

#include "cast_channel.pb-c.h"
#include "tls_client.h"
#include "cast_message.h"
#include "common.h"
#include "rfid_card.h"
#include "cjson/cJSON.h"

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
    uint8_t mediaSessionId;
    struct connectionState *cs;
    int8_t pingCount;
    // bool cardReady;
    struct RfidCard *rfidCard;
};

struct Message {
    char *msg;
    size_t msgLen;
};

void waitCard(struct CastConnectionState *self);
// void addMessage(struct CastConnectionState *self, struct Message *msg);
void addMessage(struct CastConnectionState *self, enum CastMessageType msgType);
void initCastConnectionState(struct CastConnectionState *self);
void CastConnect(struct CastConnectionState *self);

extern struct CastConnectionState cast;

#endif

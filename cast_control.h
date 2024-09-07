#ifndef _CAST_CONTROL_H
#define _CAST_CONTROL_H

#include <stdint.h>
#include <stdbool.h>

#define HEARTBEAT_NS           "urn:x-cast:com.google.cast.tp.heartbeat"
#define CONNECTION_NS          "urn:x-cast:com.google.cast.tp.connection"
#define RECEIVER_NS            "urn:x-cast:com.google.cast.receiver"
#define MEDIA_NS               "urn:x-cast:com.google.cast.media"

#define DEFAULT_SOURCE_ID      "sender-0"
#define DEFAULT_DESTINATION_ID "receiver-0"

#define APPBACKDROP            "E8C28D3C"
#define APPMEDIA               "CC1AD845"
#define APPURL                 "5CB45E5A"
#define APPYOUTUBEMUSIC        "2DB7CC49"
#define APPYOUTUBE             "233637DE"

enum CastMessageType {
    NONE,
    PING,
    PONG,
    CONNECT,
    CLOSE,
    GET_STATUS,
    RECEIVER_STATUS,
    LAUNCH,
    LAUNCH_STATUS,
    MEDIA_STATUS,
    GET_MEDIA_STATUS,
    LOAD,
    QUEUE_INSERT,
    QUEUE_NEXT,
    QUEUE_PREV,
    PLAY,
    PAUSE,
    STOP,
    SET_VOLUME
};

#define TYPE_TO_STR(type) \
    ((type) == NONE ? "NONE" : \
    (type) == PING ? "PING" : \
    (type) == PONG ? "PONG" : \
    (type) == CONNECT ? "CONNECT" : \
    (type) == CLOSE ? "CLOSE" : \
    (type) == GET_STATUS ? "GET_STATUS" : \
    (type) == RECEIVER_STATUS ? "RECEIVER_STATUS" : \
    (type) == LAUNCH ? "LAUNCH" : \
    (type) == LAUNCH_STATUS ? "LAUNCH_STATUS" : \
    (type) == MEDIA_STATUS ? "MEDIA_STATUS" : \
    (type) == GET_MEDIA_STATUS ? "GET_MEDIA_STATUS" : \
    (type) == LOAD ? "LOAD" : \
    (type) == QUEUE_INSERT ? "QUEUE_INSERT" : \
    (type) == QUEUE_NEXT ? "QUEUE_NEXT" : \
    (type) == QUEUE_PREV ? "QUEUE_PREV" : \
    (type) == PLAY ? "PLAY" : \
    (type) == PAUSE ? "PAUSE" : \
    (type) == STOP ? "STOP" : \
    (type) == SET_VOLUME ? "SET_VOLUME" : "UNKNOWN_TYPE")

enum PlayerState {
    IDLE,
    BUFFERING,
    PAUSED,
    PLAYING
};

struct Volume {
    float level;
    bool muted;
};

struct MediaStatus {
    int mediaSessionId;
    enum PlayerState playerState;
    struct Volume volume;
};
struct MessageItem {
    uint8_t *msg;
    uint16_t msgLen;
    enum CastMessageType type;
};

struct MessageData {
    char **items;
    uint16_t size;
};

struct MessageQueueItem {
    struct MessageQueueItem *next;
    enum CastMessageType type;
    struct MessageData  *data;
    enum CastMessageType waitFor;
};

struct CastState {
    char *senderId;
    char receiverId[64];
    char *appId;
    uint16_t requestId;
    struct MediaStatus mediaStatus;
    uint16_t flagType;
};

bool sendMessage(struct MessageQueueItem **item,
        struct MessageItem *msgItem, struct CastState *cs);
void queueMessage(struct MessageQueueItem **item, enum CastMessageType msgType,
        struct MessageData *data, enum CastMessageType waitFor);
enum CastMessageType processData(struct CastState *cs, uint8_t *buf);

#endif


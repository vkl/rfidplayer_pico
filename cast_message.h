#ifndef _CAST_MESSAGE_H
#define _CAST_MESSAGE_H

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
    PING = 1,
    PONG,
    CONNECT,
    CLOSE,
    GET_STATUS,
    RECEIVER_STATUS,
    LAUNCH,
    MEDIA_STATUS,
    GET_MEDIA_STATUS,
    LOAD,
    QUEUE_INSERT, 
    STOP,
    SET_VOLUME
};

struct Payload {
    enum CastMessageType Type;
    uint8_t RequestId;
};

struct Application {
    char *AppID;
	char *DisplayName;
	struct Namespace **Namespaces;
	char *SessionID;
	char *StatusText;
	char *TransportId;
};

struct Namespace {
    char *Name;
};

struct Volume {
    char *controlType;
    float level;
    bool muted;
    float stepInterval;
};

struct ReceiverStatus {
    struct Application **applications;
    struct Volume volume;
};

char *payload(enum CastMessageType msgType);

#endif
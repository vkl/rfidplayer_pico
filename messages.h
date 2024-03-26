#ifndef MESSAGES_H
#define MESSAGES_H

#define PING_PAYLOAD "{\"type\":\"PING\"}"

#define PONG_PAYLOAD "{\"type\":\"PONG\"}"

#define GET_STATUS_PAYLOAD "{\"type\":\"GET_STATUS\",\"requestId\":%d}"

#define CONNECT_PAYLOAD "{\"type\":\"CONNECT\"}"

#define RECEIVER_STATUS_PAYLOAD "{\"type\":\"RECEIVER_STATUS\",\"requestId\":%d}"

#define LAUNCH_PAYLOAD "{\"type\":\"LAUNCH\",\"requestId\":%d,\"appId\":\"%s\"}"

#define MEDIA_STATUS_PAYLOAD "{\"type\":\"MEDIA_STATUS\",\"requestId\":%d}"

#define LOAD_PAYLOAD "{\"type\":\"LOAD\",\"requestId\":%d,\
\"media\":{\"contentId\":\"%s\",\"streamType\":\"%s\",\"contentType\":\"%s\"}}"

#define STOP_PAYLOAD "{\"type\":\"STOP\",\"requestId\":%d,\"mediaSessionId\":%d}"

#endif // MESSAGES_H
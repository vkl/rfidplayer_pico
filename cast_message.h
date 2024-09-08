#ifndef _CAST_MESSAGE_H
#define _CAST_MESSAGE_H

#define PING_PAYLOAD "{\"type\":\"PING\"}"

#define PONG_PAYLOAD "{\"type\":\"PONG\"}"

#define GET_STATUS_PAYLOAD "{\"type\":\"GET_STATUS\",\"requestId\":%d}"

#define CONNECT_PAYLOAD "{\"type\":\"CONNECT\"}"

#define CLOSE_PAYLOAD "{\"type\":\"CLOSE\"}"

#define RECEIVER_STATUS_PAYLOAD "{\"type\":\"RECEIVER_STATUS\",\"requestId\":%d}"

#define LAUNCH_PAYLOAD "{\"type\":\"LAUNCH\",\"requestId\":%d,\"appId\":\"%s\"}"

#define MEDIA_STATUS_PAYLOAD "{\"type\":\"GET_STATUS\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define LOAD_PAYLOAD "{\"type\":\"LOAD\",\"requestId\":%d,\"media\":%s}"

#define QUEUE_INSERT_PAYLOAD "{\"type\":\"QUEUE_INSERT\",\"requestId\":%d,\"items\":[%s],\"mediaSessionId\":%d,\"currentTime\":%d,\"autoPlay\":%s}"

#define QUEUE_NEXT_PAYLOAD "{\"type\":\"QUEUE_NEXT\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define QUEUE_PREV_PAYLOAD "{\"type\":\"QUEUE_PREV\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define STOP_PAYLOAD "{\"type\":\"STOP\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define PAUSE_PAYLOAD "{\"type\":\"PAUSE\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define PLAY_PAYLOAD "{\"type\":\"PLAY\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define MEDIA_ITEM "{\"media\":%s,\"autoPlay\":%s,\"startTime\":%d,\"preloadTime\":%d}"

#define SET_VOLUME_PAYLOAD "{\"type\":\"SET_VOLUME\",\"requestId\":%d,\"volume\":{\"level\":%f,\"muted\":false}}"

#endif

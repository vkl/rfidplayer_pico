#ifndef MESSAGES_H
#define MESSAGES_H

#define PING_PAYLOAD "{\"type\":\"PING\"}"

#define PONG_PAYLOAD "{\"type\":\"PONG\"}"

#define GET_STATUS_PAYLOAD "{\"type\":\"GET_STATUS\",\"requestId\":%d}"

#define CONNECT_PAYLOAD "{\"type\":\"CONNECT\"}"

#define RECEIVER_STATUS_PAYLOAD "{\"type\":\"RECEIVER_STATUS\",\"requestId\":%d}"

#define LAUNCH_PAYLOAD "{\"type\":\"LAUNCH\",\"requestId\":%d,\"appId\":\"%s\"}"

#define MEDIA_STATUS_PAYLOAD "{\"type\":\"MEDIA_STATUS\",\"requestId\":%d}"

#define LOAD_PAYLOAD "{\"type\":\"LOAD\",\"requestId\":%d,\"media\":%s}"

#define QUEUE_INSERT_PAYLOAD "{\"type\":\"QUEUE_INSERT\",\"requestId\":%d,\"items\":[%s],\"mediaSessionId\":%d,\"currentTime\":%d,\"autoPlay\":%s}"

#define STOP_PAYLOAD "{\"type\":\"STOP\",\"requestId\":%d,\"mediaSessionId\":%d}"

#define MEDIA_ITEM "{\"media\":%s,\"autoPlay\":%s,\"startTime\":%d,\"preloadTime\":%d}"

#endif // MESSAGES_H
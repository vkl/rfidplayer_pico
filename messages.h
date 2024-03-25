#ifndef MESSAGES_H
#define MESSAGES_H

#define PING_ "{\"type\":\"PING\"}"

#define PONG_ "{\"type\":\"PONG\"}"

#define GET_STATUS_ "{\"type\":\"GET_STATUS\",\"requestId\":%d}"

#define CONNECT_ "{\"type\":\"CONNECT\",\"requestId\":%d}"

#define RECEIVER_STATUS_ "{\"type\":\"RECEIVER_STATUS\",\"requestId\":%d,\"sessionId\":\"%s\"}"

#define LAUNCH_ "{\"type\":\"LAUNCH\",\"requestId\":%d,\"appId\":\"%s\"}"

#define MEDIA_STATUS_ "{\"type\":\"MEDIA_STATUS\",\"requestId\":%d,\"sessionId\":\"%s\"}"

#define LOAD_ "{\"type\":\"LOAD\",\"requestId\":%d,\"sessionId\":\"%s\",\
\"media\":{\"contentId\":\"%s\",\"streamType\":\"%s\",\"contentType\":\"%s\"}}"


#endif // MESSAGES_H
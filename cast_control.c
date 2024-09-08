#include <lwip/def.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include "cjson/cJSON.h"
#include "proto/cast_channel.pb-c.h"

#include "cast_message.h"
#include "cast_control.h"

enum CastMessageType
processData(struct CastState *cs, uint8_t *buf) {
    enum CastMessageType retval = NONE;
    Api__CastMessage *cast_msg;
    size_t len = (buf[0]<<24) | (buf[1]<<16) | (buf[2]<<8) | buf[3];
    //printf("data length: %d\n", len);
    cast_msg = api__cast_message__unpack(NULL, len, &buf[4]);
    printf("%s %s -> %s\n", cast_msg->source_id, cast_msg->payload_utf8, cast_msg->destination_id);
    cJSON *payload = cJSON_Parse(cast_msg->payload_utf8);
    if (payload == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            printf("Error before: %s\n", error_ptr);
        }
        goto done;
    }
    const cJSON *type = NULL;
    type = cJSON_GetObjectItemCaseSensitive(payload, "type");
    
    if (strcmp(type->valuestring, "PING") == 0) {
        retval = PING;

    } if (strcmp(type->valuestring, "LAUNCH_STATUS") == 0) {
        retval = LAUNCH_STATUS;
    
    } else if (strcmp(type->valuestring, "RECEIVER_STATUS") == 0) {
        retval = RECEIVER_STATUS;
        cJSON *status = cJSON_GetObjectItemCaseSensitive(payload, "status");
        cJSON *apps = cJSON_GetObjectItemCaseSensitive(status, "applications");
        cJSON *volume = cJSON_GetObjectItemCaseSensitive(status, "volume");
        cJSON *volumeLevel = cJSON_GetObjectItemCaseSensitive(volume, "level");
        cs->mediaStatus.volume.level = volumeLevel->valuedouble;
        cJSON *app = cJSON_GetArrayItem(apps, 0);
        cJSON *sessionId = cJSON_GetObjectItem(app, "sessionId");
        if (sessionId != NULL) {
            if (strcmp(cs->receiverId, sessionId->valuestring) != 0)
                strcpy(cs->receiverId, sessionId->valuestring);
        }

    } else if (strcmp(type->valuestring, "MEDIA_STATUS") == 0) {
        retval = MEDIA_STATUS;
        cJSON *status = cJSON_GetObjectItemCaseSensitive(payload, "status");
        cJSON *stat = cJSON_GetArrayItem(status, 0);
        if (stat == NULL) {
            cs->mediaStatus.mediaSessionId = 0; 
            goto done;
        }
        cJSON *playerState = cJSON_GetObjectItem(stat, "playerState");
        cJSON *mediaSessionId = cJSON_GetObjectItem(stat, "mediaSessionId");

        if (strcmp(playerState->valuestring, "BUFFERING") == 0) {
            cs->mediaStatus.playerState = BUFFERING;
        } else if (strcmp(playerState->valuestring, "IDLE") == 0) {
            cs->mediaStatus.playerState = IDLE;
        } else if (strcmp(playerState->valuestring, "PAUSED") == 0) {
            cs->mediaStatus.playerState = PAUSED;
        } else if (strcmp(playerState->valuestring, "PLAYING") == 0) {
            cs->mediaStatus.playerState = PLAYING;
        }

        if (mediaSessionId->valueint != cs->mediaStatus.mediaSessionId) {
            cs->mediaStatus.mediaSessionId = mediaSessionId->valueint;
        }
    } else if (strcmp(type->valuestring, "CLOSE") == 0) {
        retval = CLOSE;
        strcpy(cs->receiverId, DEFAULT_DESTINATION_ID);
        cs->mediaStatus.mediaSessionId = 0;
        cs->requestId = 1;
    } else if (strcmp(type->valuestring, "PONG") == 0) {
        retval = PONG;
    }

done:
    api__cast_message__free_unpacked(cast_msg, NULL);     
    cJSON_Delete(payload);
    cs->flagType |= (1 << retval);
    return retval;
}

bool sendMessage(struct MessageQueueItem **msgQueueItem, struct MessageItem *msgItem, struct CastState *cs) {
    
    if ((*msgQueueItem) == NULL) return false;

    bool checkType = (*msgQueueItem)->waitFor == NONE || 
        (1 << (*msgQueueItem)->waitFor) & cs->flagType;

    if (!checkType) {
        return false;
    } else {
        cs->flagType &= ~(1 << (*msgQueueItem)->waitFor);
    } 

    Api__CastMessage cast_msg = API__CAST_MESSAGE__INIT;
    cast_msg.source_id = DEFAULT_SOURCE_ID;
    size_t n;

    switch ((*msgQueueItem)->type) {
    case PING:
        cast_msg.namespace_ = HEARTBEAT_NS;
        cast_msg.payload_utf8 = strdup(PING_PAYLOAD);
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        break;
    case PONG:
        cast_msg.namespace_ = HEARTBEAT_NS;
        cast_msg.payload_utf8 = strdup(PONG_PAYLOAD);
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        break;
    case CONNECT:
        cast_msg.namespace_ = CONNECTION_NS;
        cast_msg.payload_utf8 = strdup(CONNECT_PAYLOAD);
        cast_msg.destination_id = cs->receiverId;
        cs->requestId++;
        break;
    case GET_STATUS:
        cast_msg.namespace_ = RECEIVER_NS;
        n = snprintf(NULL, 0, GET_STATUS_PAYLOAD, cs->requestId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, GET_STATUS_PAYLOAD, cs->requestId);
        cs->requestId++;
        break;
    case GET_MEDIA_STATUS:
        cast_msg.namespace_ = MEDIA_NS;
        if (cs->mediaStatus.mediaSessionId > 0) {
            n = snprintf(NULL, 0, MEDIA_STATUS_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
            cast_msg.destination_id = cs->receiverId;
            cast_msg.payload_utf8 = malloc(n+1);
            sprintf(cast_msg.payload_utf8, MEDIA_STATUS_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        } else {
            n = snprintf(NULL, 0, GET_STATUS_PAYLOAD, cs->requestId);
            cast_msg.destination_id = cs->receiverId;
            cast_msg.payload_utf8 = malloc(n+1);
            sprintf(cast_msg.payload_utf8, GET_STATUS_PAYLOAD, cs->requestId);
        }
        cs->requestId++;
        break;
    case LAUNCH:
        cast_msg.namespace_ = RECEIVER_NS;
        n = snprintf(NULL, 0, LAUNCH_PAYLOAD, cs->requestId, cs->appId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, LAUNCH_PAYLOAD, cs->requestId, cs->appId);
        cs->requestId++;
        break;
    case CLOSE:
        cast_msg.namespace_ = CONNECTION_NS;
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = strdup(CLOSE_PAYLOAD);
        cs->requestId++;
        break;
    case LOAD:
        if ((*msgQueueItem)->data->size == 0)
            break;
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, LOAD_PAYLOAD, cs->requestId, (*msgQueueItem)->data->items[0]);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, LOAD_PAYLOAD, cs->requestId, (*msgQueueItem)->data->items[0]);
        cs->requestId++;
        break;
    case QUEUE_INSERT:
        cast_msg.namespace_ = MEDIA_NS;
        char *mediaItem = NULL;
        for (int i = 1; i < (*msgQueueItem)->data->size; i++) {
            n = snprintf(NULL, 0, MEDIA_ITEM, (*msgQueueItem)->data->items[i], "true", 0, 0);
            if (mediaItem == NULL) {
                mediaItem = malloc(n+1);
                sprintf(mediaItem, MEDIA_ITEM, (*msgQueueItem)->data->items[i], "true", 0, 0);
            } else {
                char *tmp = malloc(strlen(mediaItem) + n + 1);
                char *newItem = malloc(n+1);
                sprintf(newItem, MEDIA_ITEM, (*msgQueueItem)->data->items[i], "true", 0, 0);
                sprintf(tmp, "%s,%s", mediaItem, newItem);
                free(mediaItem);
                free(newItem);
                mediaItem = tmp;
            }
        }
        n = snprintf(NULL, 0, QUEUE_INSERT_PAYLOAD,
                cs->requestId, mediaItem, cs->mediaStatus.mediaSessionId, 0, "true");
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, QUEUE_INSERT_PAYLOAD,
                cs->requestId, mediaItem, cs->mediaStatus.mediaSessionId, 0, "true");
        cs->requestId++;
        free(mediaItem);
        break;
    case STOP:
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, STOP_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, STOP_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        break;
    case PAUSE:
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, PAUSE_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, PAUSE_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        break;
    case PLAY:
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, PLAY_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, PLAY_PAYLOAD, cs->requestId, cs->mediaStatus.mediaSessionId);
        break;
    case SET_VOLUME:
        cast_msg.namespace_ = RECEIVER_NS;
        n = snprintf(NULL, 0, SET_VOLUME_PAYLOAD, cs->requestId,
                (float)(cs->mediaStatus.volume.level));
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, SET_VOLUME_PAYLOAD, cs->requestId,
            (float)(cs->mediaStatus.volume.level));
        cs->requestId++;
        break;
    case QUEUE_NEXT:
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, QUEUE_NEXT_PAYLOAD, cs->requestId,
                cs->mediaStatus.mediaSessionId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, QUEUE_NEXT_PAYLOAD, cs->requestId,
                cs->mediaStatus.mediaSessionId);
        cs->requestId++;
        break;
    case QUEUE_PREV:
        cast_msg.namespace_ = MEDIA_NS;
        n = snprintf(NULL, 0, QUEUE_PREV_PAYLOAD, cs->requestId,
                cs->mediaStatus.mediaSessionId);
        cast_msg.destination_id = cs->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, QUEUE_PREV_PAYLOAD, cs->requestId,
                cs->mediaStatus.mediaSessionId);
        cs->requestId++;
        break;
    default:
        break;
    }
    cast_msg.protocol_version = API__CAST_MESSAGE__PROTOCOL_VERSION__CASTV2_1_0;
    cast_msg.payload_type = API__CAST_MESSAGE__PAYLOAD_TYPE__STRING;
    printf("%s -> %s %.100s\n", cast_msg.source_id, cast_msg.destination_id, cast_msg.payload_utf8);
    size_t size = api__cast_message__get_packed_size(&cast_msg);
   
    msgItem->type = (*msgQueueItem)->type;
    msgItem->msg = malloc(size+4);
    if (msgItem->msg == NULL) {
        printf("malloc error for MessageItem->msg\n");
        goto end;
    }
    *(uint32_t*)(msgItem)->msg = htonl(size);
    api__cast_message__pack(&cast_msg, &(msgItem)->msg[4]);
    //printf("send message size: %d\n", size+4);
    msgItem->msgLen = size+4;
    if ((*msgQueueItem)->next != NULL) {
        struct MessageQueueItem *tmp = (*msgQueueItem)->next;
        free((*msgQueueItem));
        (*msgQueueItem) = tmp;
    } else {
        free((*msgQueueItem));
        (*msgQueueItem) = NULL;
    }
end:
    free(cast_msg.payload_utf8);
    return true;
}

void
queueMessage(struct MessageQueueItem **item, enum CastMessageType msgType,
        struct MessageData *data, enum CastMessageType waitFor) {
    //printf("add message '%s' to queue\n", TYPE_TO_STR(msgType));
    if (*item == NULL) {
        (*item) = (struct MessageQueueItem*)calloc(1, sizeof(struct MessageQueueItem));
        if ((*item) == NULL) {
            printf("calloc error for MessageQueueItem\n");
            return;
        }
        (*item)->type = msgType;
        (*item)->waitFor = waitFor;
        if (data != NULL)
            (*item)->data = data;
        (*item)->next = NULL;
    } else {
        struct MessageQueueItem *tmp = (*item);
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = (struct MessageQueueItem*)calloc(1, sizeof(struct MessageQueueItem));
        if (tmp->next == NULL) {
            printf("calloc error for MessageQueueItem->next\n");
            return;
        }
        tmp->next->type = msgType;
        tmp->next->waitFor = waitFor;
        if (data != NULL)
            tmp->next->data = data;
        tmp->next->next = NULL;
    }
}


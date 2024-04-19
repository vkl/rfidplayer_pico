#include "cast_controllers.h"
#include "casts.h"
#include "messages.h"

#include "quadrature_encoder.pio.h"

extern PIO pio;
extern const uint sm;
// int8_t volume = 0;
int old_value = 0;
int new_value = 0;
int delta_value = 0;


void processingData(struct CastConnectionState **self, unsigned char *data) {
    struct CastConnectionState *ccs = *self;
    Api__CastMessage *cast_msg;
    size_t len = (data[0]<<24) | (data[1]<<16) | (data[2]<<8) | data[3];
    cast_msg = api__cast_message__unpack(NULL, len, &data[4]);
    DEBUG_PRINT("<- %s\n", cast_msg->payload_utf8);
    if (cast_msg->payload_utf8 == NULL) {
        return;
    }
    cJSON *payload = cJSON_Parse(cast_msg->payload_utf8);
    if (payload == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            DEBUG_PRINT("Error before: %s\n", error_ptr);
        }
        return;
    }
    const cJSON *type = NULL;
    type = cJSON_GetObjectItemCaseSensitive(payload, "type");
    
    if (strcmp(type->valuestring, "PONG") == 0) {
        ccs->pingCount--;
    
    } else if (strcmp(type->valuestring, "PING") == 0) {
        addMessage(ccs, PONG);
    
    } else if (strcmp(type->valuestring, "MEDIA_STATUS") == 0) {
        cJSON *status = cJSON_GetObjectItemCaseSensitive(payload, "status");
        cJSON *stat = cJSON_GetArrayItem(status, 0);
        if (stat == NULL) {
            addMessage(ccs, GET_MEDIA_STATUS);
            goto done;
        }
        cJSON *playerState = cJSON_GetObjectItem(stat, "playerState");
        cJSON *mediaSessionId = cJSON_GetObjectItem(stat, "mediaSessionId");
        DEBUG_PRINT("playerState %s\n", playerState->valuestring);
        DEBUG_PRINT("mediaSessionId %d\n", mediaSessionId->valueint);
        if (mediaSessionId->valueint != ccs->mediaSessionId) {
            ccs->mediaSessionId = mediaSessionId->valueint;
            if (ccs->rfidCard->MediaCount > 1) {
                addMessage(ccs, QUEUE_INSERT);
            }
        }
    
    } else if (strcmp(type->valuestring, "RECEIVER_STATUS") == 0) {
        cJSON *status = cJSON_GetObjectItemCaseSensitive(payload, "status");
        cJSON *apps = cJSON_GetObjectItemCaseSensitive(status, "applications");
        cJSON *volume = cJSON_GetObjectItemCaseSensitive(status, "volume");
        cJSON *volumeLevel = cJSON_GetObjectItemCaseSensitive(volume, "level");
        DEBUG_PRINT("receiver status volume %f\n", volumeLevel->valuedouble);
        cJSON *app = cJSON_GetArrayItem(apps, 0);
        cJSON *sessionId = cJSON_GetObjectItem(app, "sessionId");
        if (sessionId != NULL) {
            DEBUG_PRINT("receiverId %s\n", ccs->receiverId);
            DEBUG_PRINT("sessionId %s\n", sessionId->valuestring);
            if (strcmp(ccs->receiverId, sessionId->valuestring) == 0) {
                goto done;
            }
            ccs->receiverId = malloc(strlen(sessionId->valuestring) + 1);
            if (ccs->receiverId != NULL) {
                strcpy(ccs->receiverId, sessionId->valuestring);
                addMessage(ccs, CONNECT);
                addMessage(ccs, LOAD);
                addMessage(ccs, GET_MEDIA_STATUS);
            }
        } else {
            addMessage(ccs, LAUNCH);
            addMessage(ccs, GET_STATUS);
        }
    } else if (strcmp(type->valuestring, "CLOSE") == 0) {
        addMessage(ccs, CONNECT);
    }
done:
    api__cast_message__free_unpacked(cast_msg, NULL);
    cJSON_Delete(payload);
    payload = NULL;
}

void addMessage(struct CastConnectionState *self, enum CastMessageType msgType) {
    Api__CastMessage cast_msg = API__CAST_MESSAGE__INIT;
    cast_msg.source_id = DEFAULT_SOURCE_ID;
    size_t n;
    switch (msgType) {
    case PING:
        cast_msg.namespace_ = HEARTBEAT_NS;
        cast_msg.payload_utf8 = malloc(strlen(PING_PAYLOAD)+1);
        strcpy(cast_msg.payload_utf8, PING_PAYLOAD);
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        break;
    case PONG:
        cast_msg.namespace_ = HEARTBEAT_NS;
        cast_msg.payload_utf8 = malloc(strlen(PONG_PAYLOAD)+1);
        strcpy(cast_msg.payload_utf8, PONG_PAYLOAD);
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        break;
    case CONNECT:
        cast_msg.namespace_ = CONNECTION_NS;
        cast_msg.payload_utf8 = malloc(strlen(CONNECT_PAYLOAD)+1);
        strcpy(cast_msg.payload_utf8, CONNECT_PAYLOAD);
        cast_msg.destination_id = self->receiverId;
        self->requestId++;
        break;
    case GET_STATUS:
        cast_msg.namespace_ = RECEIVER_NS;
        n = sprintf(NULL, GET_STATUS_PAYLOAD, self->requestId);
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, GET_STATUS_PAYLOAD, self->requestId);
        self->requestId++;
        break;
    case GET_MEDIA_STATUS:
        cast_msg.namespace_ = MEDIA_NS;
        n = sprintf(NULL, GET_STATUS_PAYLOAD, self->requestId);
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, GET_STATUS_PAYLOAD, self->requestId);
        self->requestId++;
        break;
    case LAUNCH:
        cast_msg.namespace_ = RECEIVER_NS;
        n = sprintf(NULL, LAUNCH_PAYLOAD, self->requestId, self->appId);
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, LAUNCH_PAYLOAD, self->requestId, self->appId);
        self->requestId++;
        break;
    case LOAD:
        cast_msg.namespace_ = MEDIA_NS;
        n = sprintf(NULL, LOAD_PAYLOAD, self->requestId, self->rfidCard->Media[0]);
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        DEBUG_PRINT("media %s\n", self->rfidCard->Media[0]);
        sprintf(cast_msg.payload_utf8, LOAD_PAYLOAD, self->requestId, self->rfidCard->Media[0]);
        self->requestId++;
        break;
    case QUEUE_INSERT:
        cast_msg.namespace_ = MEDIA_NS;
        char *mediaItem = NULL;
        for (uint16_t i = 1; i < self->rfidCard->MediaCount; i++) {
            n = sprintf(NULL, MEDIA_ITEM, self->rfidCard->Media[i], "true", 0, 0);
            if (mediaItem == NULL) {
                mediaItem = malloc(n+1);
                sprintf(mediaItem, MEDIA_ITEM, self->rfidCard->Media[i], "true", 0, 0);
            } else {
                char *tmp = malloc(strlen(mediaItem) + n + 1);
                char *newItem = malloc(n+1);
                sprintf(newItem, MEDIA_ITEM, self->rfidCard->Media[i], "true", 0, 0);
                sprintf(tmp, "%s,%s", mediaItem, newItem);
                free(mediaItem);
                free(newItem);
                mediaItem = tmp;
            }
        }
        n = sprintf(NULL, QUEUE_INSERT_PAYLOAD, self->requestId, mediaItem, self->mediaSessionId, 0, "true");
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, QUEUE_INSERT_PAYLOAD, self->requestId, mediaItem, self->mediaSessionId, 0, "true");
        self->requestId++;
        free(mediaItem);
        break;
    case STOP:
        cast_msg.namespace_ = MEDIA_NS;
        n = sprintf(NULL, STOP_PAYLOAD, self->requestId, self->mediaSessionId);
        cast_msg.destination_id = self->receiverId;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, STOP_PAYLOAD, self->requestId, self->mediaSessionId);
        break;
    case SET_VOLUME:
        cast_msg.namespace_ = RECEIVER_NS;
        n = sprintf(NULL, SET_VOLUME_PAYLOAD, self->requestId, (float)(self->volume/100.0));
        cast_msg.destination_id = DEFAULT_DESTINATION_ID;
        cast_msg.payload_utf8 = malloc(n+1);
        sprintf(cast_msg.payload_utf8, SET_VOLUME_PAYLOAD, self->requestId, (float)(self->volume/100.0));
        self->requestId++;
        break;
    }
    cast_msg.protocol_version = API__CAST_MESSAGE__PROTOCOL_VERSION__CASTV2_1_0;
    cast_msg.payload_type = API__CAST_MESSAGE__PAYLOAD_TYPE__STRING;
    DEBUG_PRINT("-> %s %s\n", cast_msg.destination_id, cast_msg.payload_utf8);
    size_t size = api__cast_message__get_packed_size(&cast_msg);
    unsigned char *packed = malloc(size+4);
    *(uint32_t*)packed = htonl(size);
    api__cast_message__pack(&cast_msg, &packed[4]);
    free(cast_msg.payload_utf8);
    sem_acquire_blocking(&semafore);
    if (self->cs->item == NULL) {
        self->cs->item = (struct MessageItem*)malloc(sizeof(struct MessageItem));
        if (self->cs->item == NULL) {
            DEBUG_PRINT("malloc error");
            return;
        }
        self->cs->item->msg = packed;
        self->cs->item->msgLen = size+4;
        self->cs->item->next = NULL;
    } else {
        struct MessageItem *tmp = self->cs->item;
        while(tmp->next != NULL) {
            tmp = tmp->next;
        }
        tmp->next = (struct MessageItem*)malloc(sizeof(struct MessageItem));
        if (tmp->next == NULL) {
            DEBUG_PRINT("malloc error");
            return;
        }
        tmp->next->msg = packed;
        tmp->next->msgLen = size+4;
        tmp->next->next = NULL;
    }
    sem_release(&semafore);
}

void initCastConnectionState(struct CastConnectionState *self) {
    self->requestId = 1;
    self->senderId = DEFAULT_SOURCE_ID;
    self->receiverId = DEFAULT_DESTINATION_ID;
    self->appId = APPMEDIA;
    self->cs = NULL;
    self->pingCount = 0;
    self->cardEvent = UNKNOWN;
    self->rfidCard = NULL;
}

void waitCard(struct CastConnectionState *self) {
    while(self->cardEvent != READY || self->rfidCard == NULL) {
        sleep_ms(100);
    }
}

void CastConnect(struct CastConnectionState *self) {
    struct ChromeCast *cc = NULL;
    while(cc == NULL) {
        cc = findChromecastByName(casts, 2, self->rfidCard->ChromeCastName);
        sleep_ms(100);
    }
    self->cs = doConnect(cc->IPAddr, cc->Port);
    addMessage(self, PING);
    addMessage(self, CONNECT);
    addMessage(self, GET_STATUS);

    int count = 0;
    int state = 0;
    enum CastMessageType msgType = 0;
    while((state = pollConnection(&self->cs)) != 0) {
        if (self->cardEvent == UNKNOWN) {
            // need to wait while all messages sent
            if (self->cs->item != NULL) {
                sleep_ms(100);
                continue;
            }
            self->receiverId = DEFAULT_DESTINATION_ID;
            self->requestId = 1;
            self->mediaSessionId = 0;
            self->cs->state = CONNECTION_CLOSE;

            continue;
        } else if (self->cardEvent == REMOVED) {
            addMessage(self, STOP);
            self->cardEvent = UNKNOWN;
        }
        if (state == DATA_READY) {
            processingData(&self, self->cs->recvData);
            self->cs->state = CONNECTED;
        }
        sleep_ms(50);
        if (count > 200) {
            if (self->pingCount >= MAXFAILEDPINGS) {
                DEBUG_PRINT("The receiver didn't respond for more then %d pings\n", MAXFAILEDPINGS);
                self->cs->state = CONNECTION_CLOSE;
                continue;
            }
            count = 0;
            self->pingCount++;
            addMessage(self, PING);
        }
        // check encoder value
        new_value = quadrature_encoder_get_count(pio, sm);
        if (new_value == old_value) {
            continue;
        }
        delta_value = new_value - old_value;
        if (delta_value > 20 || delta_value < -20) {
            if (delta_value > 0) {
                self->volume = self->volume + 5;
                if (self->volume > 100) {
                    self->volume = 100;
                }
            } else if (delta_value < 0) {
                self->volume = self->volume - 5;
                if (self->volume < 0) {
                    self->volume = 0;
                }
            }
            old_value = new_value;
            DEBUG_PRINT("Volume %d\n", self->volume);
            if (self->mediaSessionId != 0) {
                addMessage(self, SET_VOLUME);
            }
        }
        count++;
    }
}

void CastDisconnect(struct CastConnectionState *self) {
    self->cs = NULL;
    free(self);
}

#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <pico/time.h>

#include "cast_control.h"
#include "cast_controllers.h"
#include "casts.h"
#include "player.h"
#include "quadrature_encoder.pio.h"
#include "rfid_card.h"

extern PIO pio;
extern const uint sm;
extern struct RfidCard *rfidCard;
extern enum CardEvent cardEvent;
extern enum PlayerBtnEvent btnEvent;

int old_value = 0;
int new_value = 0;
int delta_value = 0;

static void loadMedia(struct MessageData *data, char **media, uint16_t size); 
static void freeMedia(struct MessageData *data); 

void CastConnect(ChromeCastDevices *devices) {
    struct ConnectionState *pcs;
    
    struct MessageItem msgItem;
    struct MessageQueueItem *msgQueueItem = NULL;
    struct MessageData data;
    
    struct CastState castState = {
        .senderId = DEFAULT_SOURCE_ID,
        .appId = APPMEDIA,
        .requestId = 1
    };
    strcpy(castState.receiverId, DEFAULT_DESTINATION_ID);
    
    ChromeCastDevice *device = findChromeCastDevice(devices, rfidCard->ChromeCastName);
    if (!device) {
        printf("chromecast device '%s' doesn't exist\n",
                rfidCard->ChromeCastName);
        return;
    }
    pcs = doConnect(device->ipaddr, device->hostname, device->port);
    pcs->castState = &castState;

    loadMedia(&data, rfidCard->Media, rfidCard->MediaCount);
    
    queueMessage(&msgQueueItem, CONNECT, NULL, NONE);
    queueMessage(&msgQueueItem, LAUNCH, NULL, NONE);
    queueMessage(&msgQueueItem, CONNECT, NULL, RECEIVER_STATUS);
    //queueMessage(&msgQueueItem, GET_MEDIA_STATUS, NULL, NONE);
    queueMessage(&msgQueueItem, LOAD, &data, NONE);
    if (data.size > 1) {
        queueMessage(&msgQueueItem, QUEUE_INSERT, &data, MEDIA_STATUS);
    }
    
    int count = 0;
    int state = 0;
    int flashRate = 10;
    int flashcount = 0;
    uint8_t pingCount = 3;
    
    while((state = pollConnection(&pcs, &msgQueueItem, &msgItem)) != 0) {
        
        flashcount++;
        
        if (cardEvent == UNKNOWN && msgQueueItem == NULL) {
            strcpy(castState.receiverId, DEFAULT_DESTINATION_ID);
            castState.requestId = 1;
            castState.mediaStatus.mediaSessionId = 0;
            pcs->state = CONNECTION_CLOSE;
        
        } else if (cardEvent == REMOVED) {
            freeMedia(&data);
            queueMessage(&msgQueueItem, STOP, NULL, NONE);
            queueMessage(&msgQueueItem, CLOSE, NULL, NONE);
            cardEvent = UNKNOWN;
        }

        if (state == PREPARE_DATA && sendMessage(&msgQueueItem, &msgItem, &castState)) {
            pcs->state = READY_TO_SEND;
        }

        if (state == DATA_READY) {
            if ((1 << PING) & castState.flagType) {
                queueMessage(&msgQueueItem, PONG, NULL, NONE);
                castState.flagType &= ~(1 << PING);
            }
            pcs->state = CONNECTED;
        }
        
        sleep_ms(30);
        if (count > 100) {
            /*if (pingCount >= MAXFAILEDPINGS) {
                DEBUG_PRINT("The receiver didn't respond for more then %d pings\n", MAXFAILEDPINGS);
                pcs->state = CONNECTION_CLOSE;
                continue;
            }*/
            count = 0;
            //pingCount++;
            //queueMessage(&msgQueueItem, PING, NULL, PONG);
        }

        count++;
        if (castState.mediaStatus.playerState == PLAYING) {
            gpio_put(LED_BLUE_PIN, 1);
            gpio_put(LED_GREEN_PIN, 0);
        } else if (castState.mediaStatus.playerState == BUFFERING) {
            gpio_put(LED_BLUE_PIN, 1);
            if (flashcount >= flashRate) {
                flashcount = 0;
                GPIO_TOGGLE(LED_GREEN_PIN);
            }
        } else if (castState.mediaStatus.playerState == PAUSED) {
            gpio_put(LED_GREEN_PIN, 1);
            gpio_put(LED_BLUE_PIN, 0);
        }

        // Button events
        if (btnEvent == PUSH) {
            btnEvent = 0;
            if (castState.mediaStatus.playerState == PLAYING) {
                queueMessage(&msgQueueItem, PAUSE, NULL, NONE);
            } else if (castState.mediaStatus.playerState == PAUSED) {
                queueMessage(&msgQueueItem, PLAY, NULL, NONE);
            }
        } else if (btnEvent == LONG_PUSH) {
            btnEvent = 0;
            queueMessage(&msgQueueItem, QUEUE_NEXT, NULL, NONE);
        }

        // check encoder value
        new_value = quadrature_encoder_get_count(pio, sm);
        if (new_value == old_value) {
            continue;
        }
        delta_value = new_value - old_value;
        if (delta_value > 20 || delta_value < -20) {
            if (delta_value > 0) {
                castState.mediaStatus.volume.level = castState.mediaStatus.volume.level + 0.05;
                if (castState.mediaStatus.volume.level > 1) {
                    castState.mediaStatus.volume.level = 1;
                }
            } else if (delta_value < 0) {
                castState.mediaStatus.volume.level = castState.mediaStatus.volume.level - 0.05;
                if (castState.mediaStatus.volume.level < 0) {
                    castState.mediaStatus.volume.level = 0;
                }
            }
            old_value = new_value;
            //DEBUG_PRINT("Volume %f\n", castState.mediaStatus.volume.level);
            queueMessage(&msgQueueItem, SET_VOLUME, NULL, NONE);
        }
    }
}

static void loadMedia(struct MessageData *data, char **media, uint16_t size) {
    data->size = size;
    data->items = calloc(size, sizeof(char*));
    for (uint16_t i=0; i<size; i++) {
        data->items[i] = strdup(media[i]);
    }
}

static void freeMedia(struct MessageData *data) {
    for (uint16_t i=0; i<data->size; i++) {
        free(data->items[i]);
    }
    free(data->items);
}


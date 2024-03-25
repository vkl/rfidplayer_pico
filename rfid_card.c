#include "rfid_card.h"
#include "common.h"
#include "player.h"
#include "cast_controllers.h"
#include "casts.h"

extern struct CastConnectionState cast;

static struct RfidCard *readCard();

void rfid_card_control(uint gpio, uint32_t events) {
    DEBUG_PRINT("events %d\n", events);
    if (events & (1 << 3)) {
        DEBUG_PRINT("got event rise\n");
        cast.cardReady = true;
        cast.rfidCard = readCard();
    } else if (events & (1 << 2)) {
        DEBUG_PRINT("got event fall\n");
        cast.cardReady = false;
        if (cast.rfidCard != NULL) {
            free(cast.rfidCard);
        }
    }
}

static struct RfidCard *readCard() {
    struct RfidCard *rfidCard = malloc(sizeof(struct RfidCard));
    rfidCard->Id = "1234567890";
    rfidCard->ChromeCastName = "Bedroom Speaker";
    rfidCard->MaxVolume = 0.5;
    struct Media media = {"http://radio.4duk.ru/4duk128.mp3", "audio/mp3"};
    return rfidCard;
}

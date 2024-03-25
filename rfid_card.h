#ifndef _RFID_CARD_H
#define _RFID_CARD_H

#include "hardware/gpio.h"

#define RFID_CARD_PRESENCE_PIN 2

struct Media {
    char *Url;
    char *Type;
};

struct RfidCard {
    char *Id;
    char *ChromeCastName;
    double MaxVolume;
    struct Media Media[];
};

void rfid_card_control(uint gpio, uint32_t events);

#endif
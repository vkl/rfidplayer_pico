#ifndef _PLAYER_H
#define _PLAYER_H

#define ENCODER_PINA 10
#define ENCODER_PINB 11

#define LED_RED_PIN 18
#define LED_GREEN_PIN 19
#define LED_BLUE_PIN 20

#define GPIO_TOGGLE(pin)                   \
    do {                                   \
        gpio_put((pin), !gpio_get((pin))); \
    } while(0)

void player_init();

#endif

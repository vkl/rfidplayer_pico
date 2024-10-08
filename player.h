#ifndef _PLAYER_H
#define _PLAYER_H

#define ENCODER_PINA 10
#define ENCODER_PINB 11

#define LED_RED_PIN 18
#define LED_GREEN_PIN 19
#define LED_BLUE_PIN 20

#define BTN_PIN 16

enum PlayerBtnEvent {
    PUSH = 1,
    DOUBLE_PUSH = 2,
    LONG_PUSH = 3
};

#define GPIO_TOGGLE(pin)                   \
    do {                                   \
        gpio_put((pin), !gpio_get((pin))); \
    } while(0)

void player_init();

#endif

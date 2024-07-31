#include "stdlib.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"

#include "player.h"
#include "rfid_card.h"


void player_init() {
    gpio_init_mask((1 << RFID_CARD_PRESENCE_PIN) | (1 << RFID_READER_RESET_PIN) | \
        (1 << LED_BLUE_PIN) | (1 << LED_GREEN_PIN) | (1 << LED_RED_PIN));
    
    gpio_set_dir(LED_RED_PIN, GPIO_OUT);
    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    gpio_pull_down(LED_RED_PIN);
    gpio_pull_down(LED_GREEN_PIN);
    gpio_pull_down(LED_BLUE_PIN);
    gpio_put(LED_RED_PIN, 1);
    gpio_put(LED_GREEN_PIN, 1);
    gpio_put(LED_BLUE_PIN, 1);

    gpio_set_dir(LED_GREEN_PIN, GPIO_OUT);
    gpio_set_dir(LED_BLUE_PIN, GPIO_OUT);

    gpio_set_dir(RFID_READER_RESET_PIN, GPIO_OUT);
    gpio_pull_down(RFID_READER_RESET_PIN);
    gpio_set_dir(RFID_CARD_PRESENCE_PIN, GPIO_IN);
    gpio_pull_up(RFID_CARD_PRESENCE_PIN);

    gpio_set_irq_enabled_with_callback(
        RFID_CARD_PRESENCE_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true, &rfid_card_control);
}

void set_led_color(uint8_t red, uint8_t green, uint8_t blue) {
    gpio_put(LED_RED_PIN, red);
    gpio_put(LED_GREEN_PIN, green);
    gpio_put(LED_BLUE_PIN, blue);
}

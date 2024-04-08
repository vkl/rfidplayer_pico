#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "hardware/clocks.h"

#include "common.h"
#include "setup_wifi.h"
#include "tls_client.h"
#include "cast_controllers.h"
#include "rfid_card.h"
#include "player.h"

#include "quadrature_encoder.pio.h"

PIO pio = pio0;
const uint sm = 0;

struct CastConnectionState cast;

int main()
{
    stdio_init_all();
    rfid_uart_init();

    connectWiFi(CYW43_COUNTRY_USA, WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    gpio_init_mask((1 << RFID_CARD_PRESENCE_PIN) | (1 << RFID_READER_RESET_PIN));
    gpio_set_dir(RFID_READER_RESET_PIN, GPIO_OUT);
    gpio_pull_down(RFID_READER_RESET_PIN);
    gpio_set_dir(RFID_CARD_PRESENCE_PIN, GPIO_IN);
    gpio_pull_up(RFID_CARD_PRESENCE_PIN);

    gpio_set_irq_enabled_with_callback(
        RFID_CARD_PRESENCE_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true, &rfid_card_control);
    
    pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, ENCODER_PINA, 0);
    
    initCastConnectionState(&cast);

    // main loop
    while(true) {
        waitCard(&cast);
        CastConnect(&cast);
    }

    DEBUG_PRINT("stop");

    cyw43_arch_deinit();
    return 0;
}

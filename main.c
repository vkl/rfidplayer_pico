#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"

#include "common.h"
#include "setup_wifi.h"
#include "tls_client.h"
#include "cast_controllers.h"
#include "rfid_card.h"
#include "player.h"

struct CastConnectionState cast;

int main()
{
    stdio_init_all();
    connectWiFi(CYW43_COUNTRY_USA, WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    gpio_init(RFID_CARD_PRESENCE_PIN);
    gpio_set_dir(RFID_CARD_PRESENCE_PIN, GPIO_IN);
    gpio_pull_up(RFID_CARD_PRESENCE_PIN);
    gpio_set_irq_enabled_with_callback(
        RFID_CARD_PRESENCE_PIN,
        GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
        true, &rfid_card_control);

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

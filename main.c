#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

#include "common.h"
#include "mdns_helper.h"
#include "setup_wifi.h"
#include "cast_controllers.h"
#include "rfid_card.h"
#include "player.h"

#include "quadrature_encoder.pio.h"

PIO pio = pio0;
const uint sm = 0;

enum CardEvent cardEvent = REMOVED;
struct RfidCard *rfidCard = NULL;

int main()
{

    stdio_init_all();
    rfid_uart_init();
    player_init();

    connectWiFi(CYW43_COUNTRY_USA, WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    mdns_send();

    pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, ENCODER_PINA, 0);
    
    struct CastConnectionState cast;
    
    while(true) {
        gpio_put(LED_GREEN_PIN, 1);
        gpio_put(LED_BLUE_PIN, 1);
        gpio_put(LED_RED_PIN, 0);
        while(cardEvent != READY || rfidCard == NULL) {
            sleep_ms(100);
        }
        gpio_put(LED_RED_PIN, 1);
        initCastConnectionState(&cast);
        CastConnect(&cast);
    }

    cyw43_arch_deinit();
    return 0;
}


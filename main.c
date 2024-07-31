#include <stdio.h>
#include "pico/stdlib.h"
#include "pico/cyw43_arch.h"
#include "pico/sync.h"
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

// semaphore_t semafore;

PIO pio = pio0;
const uint sm = 0;

enum CardEvent cardEvent = REMOVED;
struct RfidCard *rfidCard = NULL;

int main()
{

    // sem_init(&semafore, 1, 1);

    stdio_init_all();
    rfid_uart_init();
    player_init();

    connectWiFi(CYW43_COUNTRY_USA, WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    // gpio_init_mask((1 << RFID_CARD_PRESENCE_PIN) | (1 << RFID_READER_RESET_PIN));
    // gpio_set_dir(RFID_READER_RESET_PIN, GPIO_OUT);
    // gpio_pull_down(RFID_READER_RESET_PIN);
    // gpio_set_dir(RFID_CARD_PRESENCE_PIN, GPIO_IN);
    // gpio_pull_up(RFID_CARD_PRESENCE_PIN);

    // gpio_set_irq_enabled_with_callback(
    //     RFID_CARD_PRESENCE_PIN,
    //     GPIO_IRQ_EDGE_RISE | GPIO_IRQ_EDGE_FALL,
    //     true, &rfid_card_control);
    
    pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, ENCODER_PINA, 0);
    
    struct CastConnectionState cast;
    
    // main loop
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

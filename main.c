#include <stdlib.h> 
#include "pico/cyw43_arch.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"
#include "pico/stdio.h"

#include "casts.h"
#include "mdns_helper.h"
#include "setup_wifi.h"
#include "cast_controllers.h"
#include "rfid_card.h"
#include "player.h"
#include "common.h"

#include "quadrature_encoder.pio.h"

#define CYW43_DRV_TIMEOUT 120000000 // 2 minutes

PIO pio = pio0;
const uint sm = 0;

extern cyw43_t cyw43_state;

enum CardEvent cardEvent = REMOVED;
enum PlayerBtnEvent btnEvent = 0;
struct RfidCard *rfidCard = NULL;

static void wait_devices_discovery(void *arg);
static void networkInit(ChromeCastDevices *devices);

int main()
{
    stdio_init_all();
    rfid_uart_init();
    player_init();

    pio_add_program(pio, &quadrature_encoder_program);
    quadrature_encoder_program_init(pio, sm, ENCODER_PINA, 0);
    
    if (gpio_get(RFID_CARD_PRESENCE_PIN) == 1) {
        DEBUG_PRINT("card is presence\n");
        irq_set_enabled(UART1_IRQ, true);
        busy_wait_ms(100);
        gpio_pull_up(RFID_READER_RESET_PIN);
        gpio_put(RFID_READER_RESET_PIN, 1);
    }

    ChromeCastDevices devices;
    uint64_t startTime, currentTime;
    
    while(true) {
        gpio_put(LED_GREEN_PIN, 1);
        gpio_put(LED_BLUE_PIN, 1);
        gpio_put(LED_RED_PIN, 0);
        startTime = time_us_64();
        while(cardEvent != READY || rfidCard == NULL) {
            sleep_ms(100);
            currentTime = time_us_64();
            if (currentTime - startTime > CYW43_DRV_TIMEOUT) {
                cyw43_arch_deinit();    
                startTime = time_us_64();
            }
        }
        gpio_put(LED_RED_PIN, 1);
        networkInit(&devices);
        CastConnect(&devices);
        sleep_ms(500);
    }

    cyw43_arch_deinit();
    freeChromeCastDevices(&devices);
    return 0;
}

static void networkInit(ChromeCastDevices *devices) {

    connectWiFi(CYW43_COUNTRY_USA, WIFI_SSID, WIFI_PASSWORD, CYW43_AUTH_WPA2_AES_PSK);

    initChromeCastDevices(devices, 10);
    mdns_send(devices, wait_devices_discovery);

    char ipaddr[16] = {0};
    for (u8_t c=0; c < devices->size; c++) {
        printf("name: %s, hostname: %s, ip: %s\n\t%s\n",
                devices->chromecastDevice[c].name,
                devices->chromecastDevice[c].hostname,
                ipaddr_ntoa(&devices->chromecastDevice[c].ipaddr),
                devices->chromecastDevice[c].txtData);
        sleep_ms(100);
    }
}

static void wait_devices_discovery(void *arg) {
    struct udp_pcb *pcb = (struct udp_pcb*)arg;
    gpio_put(LED_GREEN_PIN, 1);
    gpio_put(LED_RED_PIN, 1);
    for(u16_t c=0; c < 100; c++) {
        sleep_ms(50);
        if (c % 6 == 0) 
            GPIO_TOGGLE(LED_BLUE_PIN); 
    }
    gpio_put(LED_BLUE_PIN, 1);
    udp_disconnect(pcb);
    udp_remove(pcb);
}


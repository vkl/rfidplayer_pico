#include <hardware/pio.h>
#include <hardware/timer.h>
#include <string.h>
#include <stdio.h>

#include "rfid_card.h"
#include "common.h"

// extern struct CastConnectionState cast;
extern enum CardEvent cardEvent;
extern struct RfidCard *rfidCard;

static char cardId[15];
static uint8_t cardIdIndex = 0;

#define CARDS_COUNT 3

char *media1[1] = {"{\"contentId\":\"http://radio.4duk.ru/4duk128.mp3\",\"streamType\":\"BUFFERED\",\"contentType\":\"audio/mp3\"}"};
char *media2[1] = {"{\"contentId\":\"http://hermitage.hostingradio.ru/hermitage128.mp3\",\"streamType\":\"BUFFERED\",\"contentType\":\"audio/mp3\"}"};
char *media3[3] = {
    "{\"contentId\":\"http://192.168.1.33:50002/m/NDLNA/3.m4a\",\"streamType\":\"BUFFERED\",\"contentType\":\"audio/m4a\"}",
    "{\"contentId\":\"http://192.168.1.33:50002/m/NDLNA/4.m4a\",\"streamType\":\"BUFFERED\",\"contentType\":\"audio/m4a\"}",
    "{\"contentId\":\"http://192.168.1.33:50002/m/NDLNA/5.m4a\",\"streamType\":\"BUFFERED\",\"contentType\":\"audio/m4a\"}"
};

struct RfidCard cards[CARDS_COUNT] = {
    {"3D002FDE19D5", "Bedroom speaker", 0.5, media1, 1},
    {"3D003066E883", "Bedroom speaker", 0.5, media2, 1},
    {"3D0030C96FAB", "Bedroom speaker", 0.5, media3, 3}
};

void rfid_card_control(uint gpio, uint32_t events) {
    DEBUG_PRINT("events %d\n", events);
    if (events & (1 << 3)) {
        DEBUG_PRINT("got event rise\n");
        irq_set_enabled(UART1_IRQ, true);
        cardIdIndex = 0;
        busy_wait_ms(100);
        gpio_pull_up(RFID_READER_RESET_PIN);
        gpio_put(RFID_READER_RESET_PIN, 1);
    } else if (events & (1 << 2)) {
        cardIdIndex = 0;
        DEBUG_PRINT("got event fall\n");
        gpio_pull_down(RFID_READER_RESET_PIN);
        gpio_put(RFID_READER_RESET_PIN, 0);
        cardEvent = REMOVED;
    }
}

void on_uart_rx() {
    while (uart_is_readable(UART_ID)) {
        uint8_t ch = uart_getc(UART_ID);
        if (ch == 0x02) {
            cardIdIndex = 0;
            continue;
        } else if ((ch == '\n') || (ch == '\r')) {
            cardId[cardIdIndex] = '\0';
            irq_set_enabled(UART1_IRQ, false);
            gpio_pull_down(RFID_READER_RESET_PIN);
            gpio_put(RFID_READER_RESET_PIN, 0);
            for (int i = 0; i < CARDS_COUNT; i++) {
                DEBUG_PRINT("Card ID %s %s\n", cardId, cards[i].Id);
                if (strcmp(cardId, cards[i].Id) == 0) {
                    DEBUG_PRINT("Found card\n");
                    cardEvent = READY;
                    rfidCard = &cards[i];
                }
            }
        } else {
            cardId[cardIdIndex++] = ch;
        }
    }
}

void rfid_uart_init() {
    uint baud_rate = uart_init(UART_ID, BAUD_RATE);
    DEBUG_PRINT("Actual baud rate %d\n", baud_rate);
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    irq_set_exclusive_handler(UART1_IRQ, on_uart_rx);
    uart_set_irq_enables(UART_ID, true, false);
}


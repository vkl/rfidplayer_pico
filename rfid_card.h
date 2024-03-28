#ifndef _RFID_CARD_H
#define _RFID_CARD_H

#include "hardware/gpio.h"
#include "hardware/uart.h"
#include "hardware/irq.h"
#include "pico/stdlib.h"

#define RFID_CARD_PRESENCE_PIN 2
#define RFID_READER_RESET_PIN 3

#define UART_ID uart1
#define BAUD_RATE 9600

#define UART_TX_PIN 4
#define UART_RX_PIN 5

enum CardEvent {
    UNKNOWN = 0,
    READY = 1,
    REMOVED = 2
};

struct RfidCard {
    char *Id;
    char *ChromeCastName;
    double MaxVolume;
    char **Media;
    uint16_t MediaCount;
};

void rfid_card_control(uint gpio, uint32_t events);
void rfid_uart_init();

#endif
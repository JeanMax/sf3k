#ifndef _PINCONFIG_H
#define _PINCONFIG_H

#include <pico/stdlib.h>

#define THERMO_SPI_BUS spi0
#define THERMO_SPI_BAUDRATE (4000 * 1000)

#define THERMO_SCK_PIN 24
#define THERMO_CS_PIN 22
#define THERMO_SO_PIN 21

#define SCREEN_SDA_PIN 1
#define SCREEN_SCL_PIN 2

//TODO:
#define BUTTON_UP_PIN 15
#define BUTTON_DOWN_PIN 16
#define BUTTON_LEFT_PIN 17
#define BUTTON_RIGHT_PIN 19
#define BUTTON_OK_PIN 20

//TODO:
#define RELAY_COLD_PIN 27
#define RELAY_HOT_PIN 29


static inline uint8_t pin2gpio(uint8_t pin) {
    switch (pin) {
    case 1:
        return 0;
    case 2:
        return 1;
    case 4:
        return 2;
    case 5:
        return 3;
    case 6:
        return 4;
    case 7:
        return 5;
    case 9:
        return 6;
    case 10:
        return 7;
    case 11:
        return 8;
    case 12:
        return 9;
    case 14:
        return 10;
    case 15:
        return 11;
    case 16:
        return 12;
    case 17:
        return 13;
    case 19:
        return 14;
    case 20:
        return 15;
    case 21:
        return 16;
    case 22:
        return 17;
    case 24:
        return 18;
    case 25:
        return 19;
    case 26:
        return 20;
    case 27:
        return 21;
    case 29:
        return 22;
    case 31:
        return 26;
    case 32:
        return 27;
    case 34:
        return 28;
    }
    return 42;
}

#endif

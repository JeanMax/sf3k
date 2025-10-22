#ifndef _PINCONFIG_H
#define _PINCONFIG_H

#include <pico/stdlib.h>

#define THERMO_SPI_BUS spi0
#define THERMO_SPI_BAUDRATE (4000 * 1000)

#define SCREEN_SPEED (400 * 1000)

// GPIO n°, not PIN n°...
#define THERMO_SCK_GPIO 18
#define THERMO_CS_GPIO 17
#define THERMO_SO_GPIO 16

#define SCREEN_SDA_GPIO 0
#define SCREEN_SCL_GPIO 1

#define BUTTON_UP_GPIO 13
#define BUTTON_DOWN_GPIO 12
#define BUTTON_LEFT_GPIO 14
#define BUTTON_RIGHT_GPIO 15
#define BUTTON_OK_GPIO 11

//TODO:
#define RELAY_COLD_GPIO 21
#define RELAY_HOT_GPIO 22


#endif

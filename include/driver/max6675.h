#ifndef _MAX6675_H
#define _MAX6675_H

#include <pico/stdlib.h>
#include <hardware/spi.h>


typedef struct max6675_conf  t_max6675_conf;
struct max6675_conf {
    spi_inst_t *spi_bus;
    uint32_t baudrate;
    uint8_t so_pin;
    uint8_t sck_pin;
    uint8_t cs_pin;
};

void max6675_init(const t_max6675_conf *conf);
float max6675_get_temp(const t_max6675_conf *conf);

#endif

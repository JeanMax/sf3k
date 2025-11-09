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

/** @brief  Init the max6675 converter for thermocouple type K.
 *
 * @param conf  a t_max6675_conf with the pins, spi bus, and baudrate set
 */
void max6675_init(const t_max6675_conf *conf);

/** @brief  Read temperature from the max6675.
 *
 * @param conf  a t_max6675_conf with the pins, spi bus, and baudrate set+init
 * @param temp  a float where the temperature read from the max6675 will be stored
 *
 * @return 0 if success
 */
int max6675_get_temp(const t_max6675_conf *conf, float *temp);

#endif

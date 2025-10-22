#include "max6675.h"

#include "utils/log.h"


void max6675_init(const t_max6675_conf *conf) {
    int actual_baudrate = spi_init(conf->spi_bus, conf->baudrate);
    LOG_DEBUG("baudrate: %d / %d hz", actual_baudrate, (int)conf->baudrate);
    (void)actual_baudrate;

    gpio_set_function(conf->sck_pin, GPIO_FUNC_SPI);
    gpio_set_function(conf->so_pin, GPIO_FUNC_SPI);
    /* gpio_set_function(conf->cs_pin, GPIO_FUNC_SIO); */

    gpio_init(conf->cs_pin);
    gpio_set_dir(conf->cs_pin, GPIO_OUT);
}

float max6675_get_temp(const t_max6675_conf *conf) {
    uint8_t buf[sizeof(uint16_t)] = {0};

    gpio_put(conf->cs_pin, false);

    int ret = spi_read_blocking(conf->spi_bus, 0, buf, sizeof(uint16_t));

    gpio_put(conf->cs_pin, true);

    if (ret != sizeof(uint16_t)) {
        LOG_ERROR("Couldn't read from spi (read len: %d / %d)",
                  ret, sizeof(uint16_t));
        return -42.42;
    }

    uint16_t val = (buf[0] << 8) | buf[1];

    if (val & 0x4) {
        LOG_ERROR("No thermocouple attached");
        return -42.;
    }
    int is_negative = val & 0x80000000;

    val >>= 3;
    if (is_negative) {
        val -= 4096;
    }
    return val * 0.25;
}

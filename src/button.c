#include "button.h"

#include "log.h"
#include "hardware/gpio.h"


static void gpio_callback(uint gpio, uint32_t events) {
    (void)events;
    static TickType_t tick = 0;
    TickType_t tack = xTaskGetTickCountFromISR();

    if (tack - tick < DEBOUNCE_TICK_DELAY) {
        /* LOG_DEBUG("debounce"); */
        return;
    }

    LOG_DEBUG("GPIO %u\n", gpio); //TODO: something
    tick = tack;
}

void init_button(uint8_t pin) {
    gpio_init(pin);
    gpio_pull_up(pin);

    gpio_set_irq_enabled_with_callback(pin,
                                       GPIO_IRQ_EDGE_FALL,
                                       true, &gpio_callback);
}

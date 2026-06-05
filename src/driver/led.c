#include "led.h"

#include <pico/cyw43_arch.h>


int init_led() {
    // this is actually the wifi init
    return cyw43_arch_init();
}

void led(bool state) {
    cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
}

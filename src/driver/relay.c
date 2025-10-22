#include "relay.h"

#include <hardware/gpio.h>
#include <task.h>

#include "utils/log.h"

#define TICK_TO_SEC(t) (pdTICKS_TO_MS(t) / 1000)
#define NOW() (long)TICK_TO_SEC(xTaskGetTickCount())
#define END_OF_TIME INT32_MAX
#define NEVER (INT32_MIN / 2)


int switch_relay(t_relay *relay, bool state) {
    if (relay->is_on == state) {
        LOG_WARNING("Relay already in requested state, ignore");
        return 0;
    }

    long now = NOW();

    if (state) { // ON
        if (now - relay->last_off_sec <= relay->conf.min_off_sec) {
            LOG_WARNING("Trying to turn ON relay too soon, debounce");
            return -1;
        }
        relay->last_on_sec = now;

    } else {  // OFF
        if (now - relay->last_on_sec <= relay->conf.min_on_sec) {
            LOG_WARNING("Trying to turn OFF relay too soon, debounce");
            return -1;
        }
        relay->last_off_sec = now;
    }

    gpio_put(relay->conf.pin, state);
    relay->is_on = state;
    return 0;
}


int init_relay(t_relay *relay) {
    gpio_init(relay->conf.pin);
    gpio_set_dir(relay->conf.pin, GPIO_OUT);

    gpio_put(relay->conf.pin, false);
    relay->is_on = false;
    relay->last_off_sec = NEVER;  // so you can turn it on right now
    relay->last_on_sec = NEVER;

    return 0;
}

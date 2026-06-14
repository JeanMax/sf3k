#include "relay.h"

#include <hardware/gpio.h>

#include "utils/log.h"
#include "utils/datetime.h"


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
    /* relay->last_off_sec = NEVER;  // so you can turn it on right now */
    relay->last_off_sec = NOW();  // so you can NOT turn it on right now
    relay->last_on_sec = NEVER;

    return 0;
}

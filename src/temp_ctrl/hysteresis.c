#include "hysteresis.h"

#include "utils/log.h"
#include "ui/screen.h"
#include "shared.h"


static inline e_state state_from_relays(t_relay *hot_relay, t_relay *cool_relay) {
    if (hot_relay->is_on && cool_relay->is_on) {
        panic("HEATING and COOLING at the same time");
    }

    if (hot_relay->is_on) {
        return HEAT;
    }

    if (cool_relay->is_on) {
        return COOL;
    }

    return WAIT;
}


static int handle_temperature_relays(e_state requested_state,
                                     t_relay *hot_relay, t_relay *cool_relay) {
    if (requested_state == shared__state) {
        return 0;
    }

    int ret = 42;

    if (requested_state == WAIT) {
        ret = switch_relay(hot_relay, false)
            + switch_relay(cool_relay, false);

    } else if (requested_state == HEAT) {
        ret = switch_relay(cool_relay, false);
        if (!ret) {
            ret = switch_relay(hot_relay, true);
        }

    } else if (requested_state == COOL) {
        ret = switch_relay(hot_relay, false);
        if (!ret) {
            ret = switch_relay(cool_relay, true);
        }
    }

    if (!ret) {
        shared__state = requested_state;
    } else {  // in case we got lost somehow
        shared__state = state_from_relays(hot_relay, cool_relay);
    }

    refresh_base_state();

    return ret;
}



int ctrl_temp(t_relay *hot_relay, t_relay *cool_relay) {
    float temp_diff = (float)shared__goal_temp - shared__current_temp;

    if (temp_diff > 0) {
        // it's too cold
        if (temp_diff < shared__cool_range) {
            LOG_DEBUG("trying to wait (too cold)");
            return handle_temperature_relays(WAIT, hot_relay, cool_relay);
        }

        LOG_DEBUG("trying to heat");
        return handle_temperature_relays(HEAT, hot_relay, cool_relay);
    }

    // it's too hot
    if (temp_diff * -1 < shared__hot_range) {
        LOG_DEBUG("trying to wait (too hot)");
        return handle_temperature_relays(WAIT, hot_relay, cool_relay);
    }

    LOG_DEBUG("trying to cool");
    return handle_temperature_relays(COOL, hot_relay, cool_relay);
}

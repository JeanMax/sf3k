#include "hysteresis.h"

#include "utils/log.h"
#include "utils/datetime.h"
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

#define DISABLE_OPPOSITE_RELAY_DELAY_SEC (60 * 60)
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
        if (NOW() - cool_relay->last_off_sec < DISABLE_OPPOSITE_RELAY_DELAY_SEC) {
            LOG_WARNING("Trying to heat to soon after cooling, debounce");
            return 0;
        }
        ret = switch_relay(cool_relay, false);
        if (!ret) {
            ret = switch_relay(hot_relay, true);
        }

    } else if (requested_state == COOL) {
        if (NOW() - hot_relay->last_off_sec < DISABLE_OPPOSITE_RELAY_DELAY_SEC) {
            LOG_WARNING("Trying to cool to soon after heating, debounce");
            return 0;
        }
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
    // shared protection
    float goal_temp = shared__goal_temp;
    float hot_range = shared__hot_range;
    float cool_range = shared__cool_range;
    float current_temp = shared__current_temp;

    float cooling_start = goal_temp + hot_range;
    float cooling_stop = goal_temp + hot_range * COOLING_INERTIA_RATIO;
    float heating_start = goal_temp - cool_range;
    float heating_stop = goal_temp - cool_range * HEATING_INERTIA_RATIO;
    /* LOG_DEBUG("ctrl_temp: BEFORE correction: c_start:%.2f, c_stop:%.2f, h_start:%.2f, h_stop:%.2f", */
    /*           cooling_start, cooling_stop, heating_start, heating_stop); */

    // due to our double interval hot_range / cool_range,
    // we end up with the beer temp at goal_temp + range / 2
    // -> the following should compensate it
    float cooling_correction = hot_range / 2;
    float heating_correction = cool_range / 2;
    cooling_start -= cooling_correction;
    cooling_stop -= cooling_correction;
    heating_start += heating_correction;
    heating_stop += heating_correction;
    /* LOG_DEBUG("ctrl_temp: AFTER correction: c_start:%.2f, c_stop:%.2f, h_start:%.2f, h_stop:%.2f", */
    /*           cooling_start, cooling_stop, heating_start, heating_stop); */

#ifndef NDEBUG
    if (cooling_stop < heating_start) {
        LOG_ERROR("heating will start right after cooling (c_stop:%.2f < h_start%.2f)",
                  cooling_stop, heating_start);
    }
    if (heating_stop > cooling_start) {
        LOG_ERROR("cooling will start right after heating (h_stop:%.2f > c_start:%.2f)",
                  heating_stop, cooling_start);
    }
#endif

    // it's too hot
    if (current_temp > cooling_start) {
        //start cooling
        LOG_DEBUG("ctrl_temp: trying to cool (from %.2f to %.2f)", cooling_start, cooling_stop);
        return handle_temperature_relays(COOL, hot_relay, cool_relay);
    }

    if (current_temp > cooling_stop) {
        //keep cooling if already in this state
        if (shared__state == COOL) {
            LOG_DEBUG("ctrl_temp: keep cooling (from %.2f to %.2f)", cooling_start, cooling_stop);
            return handle_temperature_relays(COOL, hot_relay, cool_relay);
        }
        LOG_DEBUG("ctrl_temp: waiting in hot_range");
        /* return handle_temperature_relays(WAIT, hot_relay, cool_relay); */
    }

    // it's too cold
    if (current_temp < heating_start) {
        //start heating
        LOG_DEBUG("ctrl_temp: trying to heat (from %.2f to %.2f)", heating_start, heating_stop);
        return handle_temperature_relays(HEAT, hot_relay, cool_relay);
    }

    if (current_temp < heating_stop) {
        //keep heating if already in this state
        if (shared__state == HEAT) {
            LOG_DEBUG("ctrl_temp: keep heating (from %.2f to %.2f)", heating_start, heating_stop);
            return handle_temperature_relays(HEAT, hot_relay, cool_relay);
        }
        LOG_DEBUG("ctrl_temp: waiting in cool_range");
        /* return handle_temperature_relays(WAIT, hot_relay, cool_relay); */
    }

    // yay
    LOG_DEBUG("ctrl_temp: temp ctrl'ed!");
    return handle_temperature_relays(WAIT, hot_relay, cool_relay);
}

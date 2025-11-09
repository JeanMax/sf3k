#ifndef _RELAY_H
#define _RELAY_H

#include <FreeRTOS.h>

#include <stdint.h>
#include <stdbool.h>


typedef struct relay_conf  t_relay_conf;
struct relay_conf {
    int32_t min_on_sec;
    int32_t min_off_sec;
    uint8_t pin;
};

typedef struct relay  t_relay;
struct relay {
    bool is_on;
    int32_t last_on_sec;
    int32_t last_off_sec;
    t_relay_conf conf;
};


/** @brief  Init the relay.
 *
 * @param relay  a t_relay with the conf part set (pin, min on and off)
 *
 * @return 0 if success
 */
int init_relay(t_relay *relay);

/** @brief  Switch the relay.
 *
 * @param relay  a t_relay with the conf part set+init
 * @param state  turn the relay on or off
 *
 * @return 0 if success
 */
int switch_relay(t_relay *relay, bool state);


#endif

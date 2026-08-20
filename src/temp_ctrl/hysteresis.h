#ifndef _HYSTERESIS_H
#define _HYSTERESIS_H

#include "driver/relay.h"

//TODO: doc
typedef struct ctrl_temp_conf  t_ctrl_temp_conf;
struct ctrl_temp_conf {
    float cooling_start;
    float cooling_stop ;
    float heating_start;
    float heating_stop;
};

//TODO: doc
#define HYSTE_DEFAULT_HOT_RANGE 1.6
#define HYSTE_DEFAULT_COOL_RANGE 1.0

//TODO: doc
#define COOLING_INERTIA_RATIO 0.75
#define HEATING_INERTIA_RATIO 0.1

// science
#define ROOM_TEMP(pi_temp) ((pi_temp) - 16)


/** @brief  Control the temperature relays.
*
* This is based on the temperatures `shared__goal_temp` and
* `shared__current_temp`, with a really basic hysteresis algorithm.
* If the the current temperature is under the goal_temp - shared__cool_range,
* heat until reaching goal_temp. Same idea for cooling.
*
* @param hot_relay  the relay config used to trigger heating
* @param cool_relay the relay config used to trigger cooling
*
* @return 0 if success (nothing to do is a success)
*/
int ctrl_temp(t_relay *hot_relay, t_relay *cool_relay);


/** @brief  TODO
*
*/
t_ctrl_temp_conf *get_ctrl_temp_conf(void);

#endif

#ifndef _HYSTERESIS_H
#define _HYSTERESIS_H

#include "driver/relay.h"


//TODO: doc
#define HYSTE_DEFAULT_HOT_RANGE 1.6
#define HYSTE_DEFAULT_COOL_RANGE 1.0

//TODO: doc
#define COOLING_INERTIA_RATIO 0.75
#define HEATING_INERTIA_RATIO 0.1


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


#endif

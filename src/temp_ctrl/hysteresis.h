#ifndef _HYSTERESIS_H
#define _HYSTERESIS_H

#include "driver/relay.h"


#define HYSTE_RANGE 1.

int ctrl_temp(t_relay *hot_relay, t_relay *cool_relay);


#endif

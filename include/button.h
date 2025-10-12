#ifndef _BUTTON_H
#define _BUTTON_H

#include <stdint.h>
#include <stdlib.h>

#define DEBOUNCE_TICK_DELAY 420


typedef enum button e_button;
enum button {
    NO_BUTTON = 0,
    UP,
    DOWN,
    LEFT,
    RIGHT,
    OK,
};


void init_button(uint8_t pin);

#endif

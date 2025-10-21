#ifndef _MENU_H
#define _MENU_H

#include <stdint.h>

#define DEBOUNCE_TICK_DELAY 250

typedef enum location e_location;
enum location {
    BASE = 0,
    MENU,
    SET_GOAL,
};

typedef int t_menu_callback(void);


void init_menu(uint8_t pin_up, uint8_t pin_down,
               uint8_t pin_left, uint8_t pin_right,
               uint8_t pin_ok);
int menu_refresh(void);

int _switch_to_set_goal(void);
int _reboot(void);
int _reboot_to_bootsel(void);
int _todo(void);

#endif

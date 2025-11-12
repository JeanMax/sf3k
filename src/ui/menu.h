#ifndef _MENU_H
#define _MENU_H

#include <stdint.h>

#define DEBOUNCE_TICK_DELAY 250

typedef enum location e_location;
enum location {
    BASE = 0,
    MENU,
    SET_GOAL,
    SET_HOT_RANGE,
    SET_COOL_RANGE,
    PI_TEMP,
    LIGHT_LEVEL,
};

typedef int t_menu_callback(void);


/** @brief  Init the buttons combo used to navigate menu
 *
 * @param pin_up  the pin used for button up
 * @param pin_down  the pin used for button down
 * @param pin_left  the pin used for button left
 * @param pin_right  the pin used for button right
 * @param pin_ok  the pin used for button ok
 *
 * @note: these pins are hardcoded in menu.c, sorry
 */
void init_menu(uint8_t pin_up, uint8_t pin_down,
               uint8_t pin_left, uint8_t pin_right,
               uint8_t pin_ok);


/** @brief  Refresh the screen with what need to be updated,
 *          based on current menu location.
 *          Don't call from interrupt.
 *
 * @return 0 if success
 *
 * @note: This might trigger a save to flash that would have been
 *        delayed in a previous callback (set in `init_menu()`).
 *        This is to avoid having to do the flash write from an
 *        interrupt... so don't call `menu_refresh()` from an interrupt!
 */
int menu_refresh(void);

#endif

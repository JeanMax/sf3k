#ifndef _LED_H
#define _LED_H

#include <stdbool.h>


/** @brief  Init the pico led.
 *
 * You need the whole wifi shebang for this, yep.
 *
 * @return 0 if success
 */
int init_led(void);

/** @brief  Switch led state.
 *
 * @param state  turn the led on or off
 */
void led(bool state);

#endif

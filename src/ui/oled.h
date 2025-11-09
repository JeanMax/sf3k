#ifndef _OLED_H
#define _OLED_H

#include <stdint.h>

#define OLED_DEFAULT_CONTRAST 127
#define OLED_INVERT 0


/** @brief  Init the oled screen.
 *
 * This is a wrapper around `ss_oled.h`, that will keep an
 * `SSOLED` struct in global.
 *
 * @param scl_pin  the pin used for scl
 * @param sda_pin  the pin used for sda
 * @param speed  the i2c speed
 *
 * @return 0 if success, otherwise the return code from `__oledInit`
 */
int init_oled(uint8_t scl_pin, uint8_t sda_pin, int32_t speed);


/** @brief  Display a string on the oled screen.
 *
 * This is a wrapper around `__oledWriteString` from `ss_oled.h`,
 * with size and coordinates restricted to what will be needed in this app.
 *
 * @param s  the string to write (SCREEN_STR_LEN_MAX == 8)
 * @param line  Y coordinate: either 0, 1 or 2
 * @param is_big  FONT_16x32 if true, otherwise FONT_16x16
 * @param is_inverted  video invert if true
 *
 * @return 0 if success, otherwise the return code from `__oledWriteString`
 */
int oled_display_string(char *s, int line, int is_big, int is_inverted);


/** @brief  Set the oled screen contrast.
 *
 * This is a wrapper around `__oledSetContrast` from `ss_oled.h`.
 *
 * @param contrast 0=off, 255=brightest
 */
void oled_set_contrast(uint8_t contrast);


/** @brief  Clear the oled screen.
 *
 * This is a wrapper around `__oledFill` from `ss_oled.h`.
 */
void oled_clear(void);


#endif

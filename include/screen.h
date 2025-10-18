#ifndef _SCREEN_H
#define _SCREEN_H

#include <stdint.h>

#define SCREEN_DEFAULT_CONTRAST 127
#define SCREEN_SPEED (400 * 1000)
#define SCREEN_INVERT 0
#define SCREEN_STR_LEN_MAX 8

#define SCREEN_HOT_CHAR "\x80"
#define SCREEN_COLD_CHAR "\x81"


int init_screen(uint8_t scl_pin, uint8_t sda_pin);
void set_screen_contrast(uint8_t contrast);
void clear_screen(void);
int display_string(char *s, int line, int is_big, int is_inverted);

#endif

#ifndef _SCREEN_H
#define _SCREEN_H

#include <stdint.h>

#define SCREEN_DEFAULT_CONTRAST 127
#define SCREEN_INVERT 0
#define SCREEN_STR_LEN_MAX 8

#define SCREEN_HOT_CHR '\x80'
#define SCREEN_COLD_CHR '\x81'


int init_screen(uint8_t scl_pin, uint8_t sda_pin, int32_t speed);
void set_screen_contrast(uint8_t contrast);
void clear_screen(void);
int display_string(char *s, int line, int is_big, int is_inverted);

int display_base_screen(void);
int refresh_base_current_temp(void);
int refresh_base_goal_temp(void);
int refresh_base_state(void);

int display_menu_screen(char *before, char *select, char *after);

int display_set_goal_screen(void);
int refresh_set_goal(int tmp_goal);

#endif

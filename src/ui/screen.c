#include "screen.h"

#include <stdio.h>

#include "shared.h"
#include "driver/ss_oled.h"


static SSOLED g_oled = {
    .oled_addr = 0x3c,
    /* .oled_wrap = 0, // ? */
    .oled_flip = 0,
    .oled_type = OLED_128x64,
    /* uint8_t *ucScreen; */
    /* uint8_t iCursorX, iCursorY; */
    /* uint8_t oled_x, oled_y; */
    /* int iScreenOffset; */
    /* .bbi2c.iSCL = scl_pin, */
    /* .bbi2c.iSDA = sda_pin, */
    .bbi2c.picoI2C = i2c_default,
};


inline int refresh_base_goal_temp() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, "goal:%3d", shared__goal_temp);
    return display_string(temp_str, 3, 0, 0);
}

inline int refresh_base_current_temp() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, "> %2.1f <", shared__current_temp);
    return display_string(temp_str, 1, 1, 0);
}

inline int refresh_base_state() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = "  SF3K  ";
    if (shared__state == COOL) {
        temp_str[0] = SCREEN_COLD_CHR;
    } else if (shared__state == HEAT) {
        temp_str[SCREEN_STR_LEN_MAX - 1] = SCREEN_HOT_CHR;
    }
    return display_string(temp_str, 0, 0, 0);
}

int display_base_screen() {
    return refresh_base_state()
        || refresh_base_current_temp()
        || refresh_base_goal_temp();
}


int display_menu_screen(char *before, char *select, char *after) {
    return refresh_base_state()
        || display_string(before, 1, 0, 0)
        || display_string(select, 2, 0, 1)
        || display_string(after, 3, 0, 0);
}


inline int refresh_set_goal(int tmp_goal) {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, " >%3d < ", tmp_goal);
    return display_string(temp_str, 1, 1, 0);
}

int display_set_goal_screen() {
    return refresh_base_state()
        || refresh_set_goal(shared__goal_temp)
        || display_string("new goal", 3, 0, 0);
}


int display_string(char *s, int line, int is_big, int is_inverted) {
    return __oledWriteString(&g_oled, 0,
                             0, line * 2,
                             s,
                             is_big ? FONT_16x32 : FONT_16x16,
                             is_inverted, 1);
}


void set_screen_contrast(uint8_t contrast) {
    __oledSetContrast(&g_oled, contrast);
}


void clear_screen() {
    __oledFill(&g_oled, 0, 1);
}


int init_screen(uint8_t scl_pin, uint8_t sda_pin, int32_t speed) {
    g_oled.bbi2c.iSCL = scl_pin;
    g_oled.bbi2c.iSDA = sda_pin;

    int ret = __oledInit(&g_oled, SCREEN_INVERT, speed);
    if (ret == OLED_NOT_FOUND) {
        return -1;
    }

    set_screen_contrast(SCREEN_DEFAULT_CONTRAST);
    clear_screen();

    return 0;
}

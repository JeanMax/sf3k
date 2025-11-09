#include "screen.h"

#include <stdio.h>

#include "oled.h"
#include "shared.h"


inline int refresh_base_goal_temp() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, "goal:%3d", shared__goal_temp);
    return oled_display_string(temp_str, 3, 0, 0);
}

inline int refresh_base_current_temp() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, "> %2.1f <", shared__current_temp);
    return oled_display_string(temp_str, 1, 1, 0);
}

inline int refresh_base_state() {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = "  SF3K  ";
    if (shared__state == COOL) {
        temp_str[0] = SCREEN_COLD_CHR;
    } else if (shared__state == HEAT) {
        temp_str[SCREEN_STR_LEN_MAX - 1] = SCREEN_HOT_CHR;
    }
    return oled_display_string(temp_str, 0, 0, 0);
}

int display_base_screen() {
    return refresh_base_state()
        || refresh_base_current_temp()
        || refresh_base_goal_temp();
}


int display_menu_screen(char *before, char *select, char *after) {
    return refresh_base_state()
        || oled_display_string(before, 1, 0, 0)
        || oled_display_string(select, 2, 0, 1)
        || oled_display_string(after, 3, 0, 0);
}


inline int refresh_set_goal(int tmp_goal) {
    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, " >%3d < ", tmp_goal);
    return oled_display_string(temp_str, 1, 1, 0);
}

int display_set_goal_screen() {
    return refresh_base_state()
        || refresh_set_goal(shared__goal_temp)
        || oled_display_string("new goal", 3, 0, 0);
}

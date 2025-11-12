#include "menu.h"

#include <FreeRTOS.h>
#include <hardware/gpio.h>
#include <pico/bootrom.h>
#include <task.h>

#include "PinConfig.h"
#include "oled.h"
#include "screen.h"
#include "shared.h"
#include "utils/log.h"
#include "utils/persist.h"

#define HYSTE_RANGE_INC 0.1

volatile e_location g_loc = BASE;
volatile bool g_need_save = false;
volatile int g_tmp_goal_temp = 0;
volatile float g_tmp_hot_range = 0;
volatile float g_tmp_cool_range = 0;

//TODO: pass these as parameters, store them... good luck with callbacks
int _switch_to_set_goal(void);
int _switch_to_set_hot_range(void);
int _switch_to_set_cool_range(void);
int _reboot(void);
int _reboot_to_bootsel(void);
int _pi_temp(void);
int _light_level(void);

#define MAX_MENU_ENTRIES 7
char g_menu_entries[MAX_MENU_ENTRIES][SCREEN_STR_LEN_MAX + 1] =  {
    "Set goal",
    "Set cool",
    "Reboot  ",
    "BootSel ",
    "Pi temp ",
    "Light % ",
    "Set hot ",
};
t_menu_callback *g_menu_fun[MAX_MENU_ENTRIES] =  {
    _switch_to_set_goal,
    _switch_to_set_cool_range,
    _reboot,
    _reboot_to_bootsel,
    _pi_temp,
    _light_level,
    _switch_to_set_hot_range,
};
volatile int g_current_entry = 0;

#define CIRCULAR_INDEX(idx)                         \
    (((idx) + MAX_MENU_ENTRIES) % MAX_MENU_ENTRIES)
#define MENU_AT(idx)                            \
    g_menu_entries[CIRCULAR_INDEX(idx)]


int _reboot() {
    LOG_INFO("Waiting to be rebooted by watchdog");
    oled_clear();
    while(42) {}
    return 0;
}

int _reboot_to_bootsel() {
    oled_clear();
    reset_usb_boot(0, 0);
    return 0;
}

int _pi_temp() {
    g_loc = PI_TEMP;
    /* oled_clear(); // careful: flickering */
    return display_pi_temp_screen();
}

int _light_level() {
    g_loc = LIGHT_LEVEL;
    /* oled_clear(); // careful: flickering */
    return display_light_level_screen();
}


////////////////////////////////////////////////////////////////////////////////


static int switch_to_base() {
    g_loc = BASE;
    /* oled_clear(); // careful: flickering */
    return display_base_screen();
}


static int switch_to_menu() {
    g_loc = MENU;
    /* oled_clear(); // careful: flickering */
    return display_menu_screen(MENU_AT(g_current_entry - 1),
                               MENU_AT(g_current_entry),
                               MENU_AT(g_current_entry + 1));
}

static int menu_next() {
    g_current_entry = CIRCULAR_INDEX(g_current_entry + 1);
    return switch_to_menu();
}

static int menu_prev() {
    g_current_entry = CIRCULAR_INDEX(g_current_entry - 1);
    return switch_to_menu();
}

static int menu_ok() {
    return g_menu_fun[g_current_entry]();
}

int menu_refresh() {
    switch(g_loc) {
    case BASE:
        if (g_need_save) {
            LOG_INFO("Saving config to flash");
            save_persistent_config();
            g_need_save = false;
        }
        return refresh_base_current_temp();
    case PI_TEMP:
        return refresh_pi_temp();
    case LIGHT_LEVEL:
        return refresh_light_level();
    default:
        return 0;
    }
}


////////////////////////////////////////////////////////////////////////////////


int _switch_to_set_hot_range() {
    g_loc = SET_HOT_RANGE;
    g_tmp_hot_range = shared__hot_range;
    /* oled_clear(); // careful: flickering */
    return display_set_hot_range_screen();
}

static int hot_range_inc() {
    g_tmp_hot_range += HYSTE_RANGE_INC;
    return refresh_set_hot_range(g_tmp_hot_range);
}

static int hot_range_dec() {
    g_tmp_hot_range -= HYSTE_RANGE_INC;
    if (g_tmp_hot_range < 0) {
        g_tmp_hot_range = 0;
    }
    return refresh_set_hot_range(g_tmp_hot_range);
}

static int hot_range_ok() {
    shared__hot_range = g_tmp_hot_range;  // TODO: might want to do that after interupt
    g_need_save = true;
    return switch_to_base();
}


////////////////////////////////////////////////////////////////////////////////


int _switch_to_set_cool_range() {
    g_loc = SET_COOL_RANGE;
    g_tmp_cool_range = shared__cool_range;
    /* oled_clear(); // careful: flickering */
    return display_set_cool_range_screen();
}

static int cool_range_inc() {
    g_tmp_cool_range += HYSTE_RANGE_INC;
    return refresh_set_cool_range(g_tmp_cool_range);
}

static int cool_range_dec() {
    g_tmp_cool_range -= HYSTE_RANGE_INC;
    if (g_tmp_cool_range < 0) {
        g_tmp_cool_range = 0;
    }
    return refresh_set_cool_range(g_tmp_cool_range);
}

static int cool_range_ok() {
    shared__cool_range = g_tmp_cool_range;  // TODO: might want to do that after interupt
    g_need_save = true;
    return switch_to_base();
}


////////////////////////////////////////////////////////////////////////////////


int _switch_to_set_goal() {
    g_loc = SET_GOAL;
    g_tmp_goal_temp = shared__goal_temp;
    /* oled_clear(); // careful: flickering */
    return display_set_goal_screen();
}

static int goal_inc() {
    return refresh_set_goal(++g_tmp_goal_temp);
}

static int goal_dec() {
    return refresh_set_goal(--g_tmp_goal_temp);
}

static int goal_ok() {
    shared__goal_temp = g_tmp_goal_temp;  // TODO: might want to do that after interupt
    g_need_save = true;
    return switch_to_base();
}


////////////////////////////////////////////////////////////////////////////////


static void on_up() {
    LOG_DEBUG("Button UP");     /* DEBUG */

    switch(g_loc) {
    case BASE:
        switch_to_menu();
        break;
    case MENU:
        menu_prev();
        break;
    case SET_GOAL:
        goal_inc();
        break;
    case SET_HOT_RANGE:
        hot_range_inc();
        break;
    case SET_COOL_RANGE:
        cool_range_inc();
        break;
    default:
        switch_to_menu();
    }
}

static void on_down() {
    LOG_DEBUG("Button DOWN");       /* DEBUG */

    switch(g_loc) {
    case BASE:
        switch_to_menu();
        break;
    case MENU:
        menu_next();
        break;
    case SET_GOAL:
        goal_dec();
        break;
    case SET_HOT_RANGE:
        hot_range_dec();
        break;
    case SET_COOL_RANGE:
        cool_range_dec();
        break;
    default:
        switch_to_menu();
    }
}

static void on_left() {
    LOG_DEBUG("Button LEFT");       /* DEBUG */

    switch(g_loc) {
    case BASE:
        switch_to_menu();
        break;
    case MENU:
        switch_to_base();
        break;
    default:
        switch_to_menu();
    }
}

static void on_right() {
    LOG_DEBUG("Button RIGHT");      /* DEBUG */

    switch(g_loc) {
    case BASE:
        switch_to_menu();
        break;
    case MENU:
        menu_ok();
        break;
    case SET_GOAL:
        goal_ok();
        break;
    case SET_HOT_RANGE:
        hot_range_ok();
        break;
    case SET_COOL_RANGE:
        cool_range_ok();
        break;
    default:
        switch_to_base();
    }
}

static void on_ok() {
    LOG_DEBUG("Button OK");     /* DEBUG */

    switch(g_loc) {
    case BASE:
        switch_to_menu();
        break;
    case MENU:
        menu_ok();
        break;
    case SET_GOAL:
        goal_ok();
        break;
    case SET_HOT_RANGE:
        hot_range_ok();
        break;
    case SET_COOL_RANGE:
        cool_range_ok();
        break;
    default:
        switch_to_base();
    }
}


static void gpio_callback(uint pin, uint32_t events) {
    (void)events;
    static TickType_t tick = 0;
    TickType_t tack = xTaskGetTickCountFromISR();

    if (tack - tick < DEBOUNCE_TICK_DELAY) {
        /* LOG_DEBUG("debounce"); */
        return;
    }
    tick = tack;

    switch(pin) {
    case BUTTON_UP_GPIO:
        on_up();
        break;
    case BUTTON_DOWN_GPIO:
        on_down();
        break;
    case BUTTON_LEFT_GPIO:
        on_left();
        break;
    case BUTTON_RIGHT_GPIO:
        on_right();
        break;
    case BUTTON_OK_GPIO:
        on_ok();
        break;
    default:
        LOG_WARNING("Unknown button pin: %u", pin);
    }
}


inline static void init_button(uint8_t pin) {
    gpio_init(pin);
    gpio_pull_up(pin);

    gpio_set_irq_enabled_with_callback(pin,
                                       GPIO_IRQ_EDGE_FALL,
                                       true, gpio_callback);
}

void init_menu(uint8_t pin_up, uint8_t pin_down,
               uint8_t pin_left, uint8_t pin_right,
               uint8_t pin_ok) {
    switch_to_base();

    //TODO: could remove the pins from params since they are
    //      hardcoded in the callback anyway
    init_button(pin_up);
    init_button(pin_down);
    init_button(pin_left);
    init_button(pin_right);
    init_button(pin_ok);
}

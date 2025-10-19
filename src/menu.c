#include "menu.h"

#include <hardware/gpio.h>
#include <FreeRTOS.h>
#include <task.h>

#include "screen.h"
#include "shared.h"
#include "PinConfig.h"
#include "log.h"
#include "persist.h"


volatile e_location g_loc = BASE;
volatile int g_need_save = 0;
volatile int g_tmp_goal_temp = 0;

#define MAX_MENU_ENTRIES 5
char g_menu_entries[MAX_MENU_ENTRIES][SCREEN_STR_LEN_MAX] =  {
    "Set goal",
    "Somethin",
    "Bla bla ",
    " Lorem  ",
    "  Ipsum ",
};
t_menu_callback *g_menu_fun[MAX_MENU_ENTRIES] =  {
    _switch_to_set_goal,
    _todo,
    _todo,
    _todo,
    _todo,
};
volatile int g_current_entry = 0;

#define CIRCULAR_INDEX(idx)                         \
    (((idx) + MAX_MENU_ENTRIES) % MAX_MENU_ENTRIES)
#define MENU_AT(idx)                            \
    g_menu_entries[CIRCULAR_INDEX(idx)]


int _todo() {
    return 0;
}


static int switch_to_base() {
    g_loc = BASE;
    /* clear_screen(); // careful: flickering */
    return display_base_screen();
}


static int switch_to_menu() {
    g_loc = MENU;
    /* clear_screen(); // careful: flickering */
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
    if (g_loc == BASE) {
        if (g_need_save) {
            LOG_INFO("Saving goal");
            save_goal_temp();
            g_need_save = 0;
        }
        return refresh_base_current_temp();
    }
    return 0;
}


int _switch_to_set_goal() {
    g_loc = SET_GOAL;
    g_tmp_goal_temp = shared__goal_temp;
    /* clear_screen(); // careful: flickering */
    return display_set_goal_screen();
}

static int goal_inc() {
    return refresh_set_goal(++g_tmp_goal_temp);
}

static int goal_dec() {
    return refresh_set_goal(--g_tmp_goal_temp);
}

static int goal_ok() {
    shared__goal_temp = g_tmp_goal_temp;
    g_need_save = 1;
    return switch_to_base();
}


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
    case SET_GOAL:
        switch_to_menu();
        break;
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
    }
}


static void gpio_callback(uint pin, uint32_t events) {
    (void)events;
    static volatile TickType_t tick = 0;
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

    init_button(pin_up);
    init_button(pin_down);
    init_button(pin_left);
    init_button(pin_right);
    init_button(pin_ok);
}

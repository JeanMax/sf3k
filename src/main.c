#include <FreeRTOS.h>
#include <hardware/watchdog.h>
#include <task.h>
#include <tusb.h>

#include "PinConfig.h"
#include "driver/led.h"
#include "driver/max6675.h"
#include "driver/relay.h"
#include "shared.h"
#include "ui/menu.h"
#include "ui/screen.h"
#include "utils/log.h"
#include "utils/persist.h"
#include "temp_ctrl/hysteresis.h"


#define REFRESH_DELAY_MS 1000

#ifdef NDEBUG
# define WATCHDOG_STOP_ON_DEBUG 0
#else
# define WATCHDOG_STOP_ON_DEBUG 1
#endif
#define WATCHDOG_DELAY_MS 1000

//TODO: move these
volatile float shared__current_temp = 0;
volatile int shared__goal_temp = 0;
volatile e_state shared__state = WAIT;


#define USB_DEFAULT_TIMEOUT_MS 3000

static int wait_for_usb(int timeout_ms) {
#ifdef NDEBUG
    return 0;
#endif
    uint sleep_inc = 100;
    for (int i = 0; i < timeout_ms; i += sleep_inc) {
        if (tud_cdc_connected()) {
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(sleep_inc));
    }
    return -1;
}


static void thermo_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 1 | 2);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("thermo task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    t_max6675_conf conf = {
        .spi_bus = THERMO_SPI_BUS,
        .baudrate = THERMO_SPI_BAUDRATE,
        .so_pin = THERMO_SO_GPIO,
        .sck_pin = THERMO_SCK_GPIO,
        .cs_pin = THERMO_CS_GPIO,
    };

    max6675_init(&conf);

    while (42) {
        shared__current_temp = max6675_get_temp(&conf); //TODO: returns status, pass &float
        LOG_INFO("Temp: %.2f°C", shared__current_temp);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void relay_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 1 | 2);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("relay task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    t_relay hot_relay = {0};
    hot_relay.conf.min_on_sec = 30;
    hot_relay.conf.min_off_sec = 30;
    init_relay(&hot_relay);

    t_relay cool_relay = {0};
    cool_relay.conf.min_on_sec = 5 * 60;
    cool_relay.conf.min_off_sec = 5 * 60;
    init_relay(&cool_relay);

    vTaskDelay(pdMS_TO_TICKS(3000));  // TODO: wait for temp

    vTaskDelay(pdMS_TO_TICKS(2000));  // in case of reboot loop

    char *state2str[] = {"WAIT", "COOL", "HEAT"};

    while (42) {
        ctrl_temp(&hot_relay, &cool_relay);
        LOG_INFO("Relay: %s", state2str[shared__state]);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void menu_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 1); // just to be extra safe on the flash side
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("menu task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    init_screen(SCREEN_SCL_GPIO,
                SCREEN_SDA_GPIO,
                SCREEN_SPEED);

    init_menu(BUTTON_UP_GPIO,
              BUTTON_DOWN_GPIO,
              BUTTON_LEFT_GPIO,
              BUTTON_RIGHT_GPIO,
              BUTTON_OK_GPIO);


    while (42) {
        menu_refresh();
        LOG_INFO("Goal: %d°C", shared__goal_temp);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void led_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 1 | 2);

    if (init_led()) {
        panic("Wi-Fi init failed");
    }
    led(true);

    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("led task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    if (watchdog_enable_caused_reboot()) {
        LOG_ERROR("Rebooted by Watchdog!");
    }
    watchdog_enable(WATCHDOG_DELAY_MS, WATCHDOG_STOP_ON_DEBUG);

    while (42) {
        led(false);
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_DELAY_MS / 2));
        watchdog_update();
        led(true);
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_DELAY_MS / 2));
        watchdog_update();
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


int main() {
    stdio_init_all();
    init_log_mutex();
    restore_goal_temp();

    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                4, NULL);
    BaseType_t relay_task_status = xTaskCreate(relay_task, "relay_task",
                                               configMINIMAL_STACK_SIZE, NULL,
                                               3, NULL);
    BaseType_t menu_task_status = xTaskCreate(menu_task, "menu_task",
                                              configMINIMAL_STACK_SIZE, NULL,
                                              2, NULL);
    BaseType_t led_task_status = xTaskCreate(led_task, "led_task",
                                             configMINIMAL_STACK_SIZE, NULL,
                                             1, NULL);

    if (
        thermo_task_status == pdPASS
        && relay_task_status == pdPASS
        && menu_task_status == pdPASS
        && led_task_status == pdPASS) {

        vTaskStartScheduler();
        // blocking
    } else {
        for (uint8_t i = 0; i < 10; i++) {
            led(true);
            vTaskDelay(pdMS_TO_TICKS(100));
            led(false);
            vTaskDelay(pdMS_TO_TICKS(100));
        }
        // panic
    }

    // we should never return from FreeRTOS
    panic_unsupported();
}

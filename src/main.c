#include <FreeRTOS.h>
#include <task.h>
#include <tusb.h>

#include "PinConfig.h"
#include "log.h"
#include "led.h"
#include "max6675.h"
#include "menu.h"
#include "screen.h"
#include "shared.h"


//TODO: move these
volatile float shared__current_temp = 0;
volatile int shared__goal_temp = 0;  //TODO: load from flash
volatile e_state shared__state = WAIT;


static int wait_for_usb(int timeout_ms) {
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
    wait_for_usb(3000);
    LOG_INFO("thermo task started");

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
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void menu_task(void *data) {
    (void)data;
    wait_for_usb(3000);
    LOG_INFO("menu task started");

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
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void led_task(void *data) {
    (void)data;

    if (init_led()) {
        panic("Wi-Fi init failed");
    }
    led(true);

    wait_for_usb(3000);
    LOG_INFO("led task started");

    while (42) {
        led(false);
        vTaskDelay(pdMS_TO_TICKS(500));
        led(true);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


int main() {
    stdio_init_all();
    init_log_mutex();

    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                configMAX_PRIORITIES - 1, NULL);
    BaseType_t menu_task_status = xTaskCreate(menu_task, "menu_task",
                                              configMINIMAL_STACK_SIZE, NULL,
                                              configMAX_PRIORITIES - 2, NULL);
    BaseType_t led_task_status = xTaskCreate(led_task, "led_task",
                                             configMINIMAL_STACK_SIZE, NULL,
                                             configMAX_PRIORITIES - 3, NULL);


    if (thermo_task_status == pdPASS
        && menu_task_status == pdPASS
        && led_task_status == pdPASS) {
        /* queue = xQueueCreate(4, sizeof(uint8_t)); */

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

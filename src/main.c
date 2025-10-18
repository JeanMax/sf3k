#include <FreeRTOS.h>
#include <task.h>

#include "PinConfig.h"
#include "log.h"
#include "led.h"
#include "max6675.h"
#include "button.h"
#include "screen.h"


volatile float g_temp = 0;


static void thermo_task(void *data) {
    (void)data;
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for usb

    t_max6675_conf conf = {
        .spi_bus = THERMO_SPI_BUS,
        .baudrate = THERMO_SPI_BAUDRATE,
        .so_pin = pin2gpio(THERMO_SO_PIN),
        .sck_pin = pin2gpio(THERMO_SCK_PIN),
        .cs_pin = pin2gpio(THERMO_CS_PIN),
    };

    max6675_init(&conf);

    while (g_temp < 1) {
        g_temp = max6675_get_temp(&conf);
        vTaskDelay(pdMS_TO_TICKS(100));
    }

    LOG_INFO("thermo task started");


    while (42) {
        g_temp = max6675_get_temp(&conf);
        LOG_INFO("Temp: %.2f°C", g_temp);
        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void button_task(void *data) {
    (void)data;
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for usb

    LOG_INFO("button task started");

    init_button(pin2gpio(BUTTON_UP_PIN));
    init_button(pin2gpio(BUTTON_DOWN_PIN));
    init_button(pin2gpio(BUTTON_LEFT_PIN));
    init_button(pin2gpio(BUTTON_RIGHT_PIN));
    init_button(pin2gpio(BUTTON_OK_PIN));

    while (42) {
        vTaskDelay(pdMS_TO_TICKS(50));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void led_task(void *data) {
    (void)data;

    if (init_led()) {
        LOG_ERROR("Wi-Fi init failed");
        panic("Wi-Fi init failed");
    }

    led(true);
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for usb

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


static void screen_task(void *data) {
    (void)data;

    LOG_INFO("screen task started");

    init_screen(pin2gpio(SCREEN_SCL_PIN), pin2gpio(SCREEN_SDA_PIN));

    display_string(" *SF3K*", 0, 0, 0);
    display_string("blabla", 1, 0, 0);
    display_string("lorem i", 2, 0, 1);
    display_string("sum truc ", 3, 0, 0);

    vTaskDelay(pdMS_TO_TICKS(2000));  // wait for usb

    char temp_str[SCREEN_STR_LEN_MAX + 1] = {0};
    snprintf(temp_str, 8, "> %.1f <", g_temp);
    clear_screen(); // careful: flickering
    /* display_string(" *SF3K*", 0, 0, 0); */
    display_string(SCREEN_HOT_CHAR " SF3K " SCREEN_COLD_CHAR, 0, 0, 0);
    display_string(temp_str, 1, 1, 0);
    display_string("goal: 36", 3, 0, 0);

    while (42) {
        snprintf(temp_str, SCREEN_STR_LEN_MAX + 1, "> %.1f <", g_temp);
        display_string(temp_str, 1, 1, 0);

        vTaskDelay(pdMS_TO_TICKS(1000));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


int main() {
    stdio_init_all();
    init_log_mutex();
    /* sleep_ms(3000);  // give usb connection some time */

    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                4, NULL);
    BaseType_t screen_task_status = xTaskCreate(screen_task, "screen_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                3, NULL);
    BaseType_t button_task_status = xTaskCreate(button_task, "button_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                2, NULL);
    BaseType_t led_task_status = xTaskCreate(led_task, "led_task",
                                             configMINIMAL_STACK_SIZE, NULL,
                                             1, NULL);


    if (thermo_task_status == pdPASS
        && button_task_status == pdPASS
        && led_task_status == pdPASS
        && screen_task_status == pdPASS) {
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

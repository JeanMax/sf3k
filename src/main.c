#include <FreeRTOS.h>
#include <task.h>

#include "PinConfig.h"
#include "log.h"
#include "led.h"
#include "max6675.h"
#include "button.h"


#define BUTTON_TASK_PRIORITY  2
#define THERMO_TASK_PRIORITY  2
#define LED_TASK_PRIORITY  2

static void thermo_task(void *data) {
    (void)data;
    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for usb

    LOG_INFO("thermo task started");

    t_max6675_conf conf = {
        .spi_bus = THERMO_SPI_BUS,
        .baudrate = THERMO_SPI_BAUDRATE,
        .so_pin = pin2gpio(THERMO_SO_PIN),
        .sck_pin = pin2gpio(THERMO_SCK_PIN),
        .cs_pin = pin2gpio(THERMO_CS_PIN),
    };

    max6675_init(&conf);

    while (42) {
        float temp = max6675_get_temp(&conf);
        LOG_INFO("Temp: %.2f°C", temp);

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


int main() {
    stdio_init_all();
    init_log_mutex();
    /* sleep_ms(3000);  // give usb connection some time */

    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                THERMO_TASK_PRIORITY, NULL);
    BaseType_t button_task_status = xTaskCreate(button_task, "button_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                BUTTON_TASK_PRIORITY, NULL);
    BaseType_t led_task_status = xTaskCreate(led_task, "led_task",
                                             configMINIMAL_STACK_SIZE, NULL,
                                             LED_TASK_PRIORITY, NULL);


    if (thermo_task_status == pdPASS
        && button_task_status == pdPASS
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

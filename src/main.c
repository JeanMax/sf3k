#include <FreeRTOS.h>
#include <task.h>

#include "PinConfig.h"
#include "log.h"
#include "led.h"
#include "max6675.h"


#define THERMO_TASK_PRIORITY  2

static void thermo_task(void *data) {
    (void)data; // unused parameter
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
        led(false);
        float temp = max6675_get_temp(&conf);
        LOG_INFO("Temp: %.2f°C", temp);

        vTaskDelay(pdMS_TO_TICKS(500));
        led(true);
        vTaskDelay(pdMS_TO_TICKS(500));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}

int main() {
    stdio_init_all();
    if (init_led()) {
        LOG_ERROR("Wi-Fi init failed");
        panic("Wi-Fi init failed");
    }
    /* sleep_ms(3000);  // give usb connection some time */

    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                THERMO_TASK_PRIORITY, NULL);


    if (thermo_task_status == pdPASS) {
        /* queue = xQueueCreate(4, sizeof(uint8_t)); */

        led(true);
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

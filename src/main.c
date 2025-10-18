#include <FreeRTOS.h>
#include <task.h>

#include "PinConfig.h"
#include "log.h"
#include "led.h"
#include "max6675.h"
#include "button.h"
#include "ss_oled.h"


#define DEFAULT_TASK_PRIORITY  2

static void thermo_task(void *data) {
    (void)data;


    t_max6675_conf conf = {
        .spi_bus = THERMO_SPI_BUS,
        .baudrate = THERMO_SPI_BAUDRATE,
        .so_pin = pin2gpio(THERMO_SO_PIN),
        .sck_pin = pin2gpio(THERMO_SCK_PIN),
        .cs_pin = pin2gpio(THERMO_CS_PIN),
    };

    max6675_init(&conf);

    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for usb
    max6675_get_temp(&conf);  // first read is always 0?
    vTaskDelay(pdMS_TO_TICKS(1000)); // wait for usb

    LOG_INFO("thermo task started");


    while (42) {
        float temp = max6675_get_temp(&conf);
        LOG_INFO("Temp: %.2f°C", temp);
        vTaskDelay(pdMS_TO_TICKS(1000)); // wait for usb
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

    vTaskDelay(pdMS_TO_TICKS(3000));
    LOG_INFO("screen task started");

    bool bInvert = 0;
    int32_t iSpeed = 400 * 1000; // 100 * 1000
#define PICO_I2C i2c0  // i2c_default
/* #define OLED_WIDTH 128 */
/* #define OLED_HEIGHT 64 */

    static uint8_t ucBuffer[1024] = {0};
    SSOLED oled = {
        .oled_addr = 0x3c,
        /* .oled_wrap = 0, // ? */
        .oled_flip = 0,
        .oled_type = OLED_128x64,
        /* uint8_t *ucScreen; */
        /* uint8_t iCursorX, iCursorY; */
        /* uint8_t oled_x, oled_y; */
        /* int iScreenOffset; */
        .bbi2c.iSCL = pin2gpio(SCREEN_SCL_PIN),
        .bbi2c.iSDA = pin2gpio(SCREEN_SDA_PIN),
        .bbi2c.picoI2C = PICO_I2C,
    };

    int ret = __oledInit(&oled, bInvert, iSpeed);
    __oledSetBackBuffer(&oled, ucBuffer);
    LOG_DEBUG("oled init: %d", ret);

    vTaskDelay(pdMS_TO_TICKS(2000));

    __oledFill(&oled, 0, 1);
    __oledSetContrast(&oled, 127);

    /* ret = __oledWriteString(&oled, 0,0,0,(char *)"**************** ", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,4,1,(char *)"Pi Pico SS_OLED", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,8,2,(char *)"running on the", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,8,3,(char *)"SSD1306 128x64", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,4,4,(char *)"monochrome OLED", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,0,5,(char *)"Written by L BANK", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,4,6,(char *)"Pico by M KOOIJ", FONT_8x8, 0, 1); */
    /* ret = __oledWriteString(&oled, 0,0,7,(char *)"**************** ", FONT_8x8, 0, 1); */
    /* vTaskDelay(pdMS_TO_TICKS(2000)); */

    __oledFill(&oled, 0, 1);
    ret = __oledWriteString(&oled,0,0,0,(char *)"Now with 5 font sizes", FONT_6x8, 0, 1);
    ret = __oledWriteString(&oled,0,0,1,(char *)"6x8 8x8 16x16", FONT_8x8, 0, 1);
    ret = __oledWriteString(&oled,0,0,2,(char *)"16x32 and a new", FONT_8x8, 0, 1);
    ret = __oledWriteString(&oled,0,0,3,(char *)"Stretched", FONT_12x16, 0, 1);
    ret = __oledWriteString(&oled,0,0,5,(char *)"from 6x8", FONT_12x16, 0, 1);
    vTaskDelay(pdMS_TO_TICKS(2000));

    __oledFill(&oled, 0, 1);
    ret = __oledWriteString(&oled,0,0,0,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);
    ret = __oledWriteString(&oled,0,0,2,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);
    ret = __oledWriteString(&oled,0,0,4,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);
    ret = __oledWriteString(&oled,0,0,6,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);


    __oledFill(&oled, 0, 1);
    ret = __oledWriteString(&oled,0,0,0,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);
    ret = __oledWriteString(&oled,0,0,2,(char *)"12345678", FONT_16x16, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);
    ret = __oledWriteString(&oled,0,0,4,(char *)"12345678", FONT_16x32, 0, 1);
    LOG_DEBUG("oled write string: %d", ret);


    while (42) {
        vTaskDelay(pdMS_TO_TICKS(1000));
        /* LOG_DEBUG("screen loop"); */
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

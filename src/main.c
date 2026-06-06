#include <FreeRTOS.h>
#include <hardware/watchdog.h>
#include <task.h>
#include <tusb.h>
#include <pico/cyw43_arch.h>

#include "PinConfig.h"
#include "driver/led.h"
/* #include "driver/max6675.h" */
#include "driver/dht.h"
#include "driver/relay.h"
#include "driver/photor.h"
#include "shared.h"
#include "temp_ctrl/hysteresis.h"
#include "temp_ctrl/mean_temp.h"
#include "ui/menu.h"
#include "ui/oled.h"
#include "utils/log.h"
#include "utils/persist.h"
#include "wifi/http_client.h"
/* #include "wifi/rmrf.fr.h" */
#include "wifi/brewersfriend.com.h"

#define REFRESH_DELAY_MS 1000

#ifdef NDEBUG
# define WATCHDOG_STOP_ON_DEBUG 0
#else
# define WATCHDOG_STOP_ON_DEBUG 1
#endif
#define WATCHDOG_DELAY_MS 1000

#define WIFI_CONNECT_TIMEOUT_MS 30000
#define HTTP_REQUEST_DELAY_MS (15 * 60 * 1000)

//TODO: move these
volatile float shared__current_temp = -42;
volatile int shared__goal_temp = 0;
volatile e_state shared__state = WAIT;
volatile float shared__hot_range = HYSTE_DEFAULT_HOT_RANGE;
volatile float shared__cool_range = HYSTE_DEFAULT_COOL_RANGE;


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
    vTaskCoreAffinitySet(NULL, 2);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("thermo task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    /* t_max6675_conf conf = { */
    /*     .spi_bus = THERMO_SPI_BUS, */
    /*     .baudrate = THERMO_SPI_BAUDRATE, */
    /*     .so_pin = THERMO_SO_GPIO, */
    /*     .sck_pin = THERMO_SCK_GPIO, */
    /*     .cs_pin = THERMO_CS_GPIO, */
    /* }; */

    /* max6675_init(&conf); */
    /* float tmp_temp = -42; */
    /* int ret = 0; */

    init_dht(DHT_GPIO);
    float tmp_temp = -42;
    float tmp_hum = -42;
    int ret = 0;

    while (42) {
        /* ret = max6675_get_temp(&conf, &tmp_temp); */
        ret = dht_read(&tmp_temp, &tmp_hum, DHT_GPIO);
        if (!ret) {
            add_temp_to_history(tmp_temp);
            shared__current_temp = get_mean_temp();
            LOG_INFO("Temp: %.2f°C - Mean: %.2f°C", tmp_temp, shared__current_temp);
            LOG_DEBUG("Humidity: %d%%", (int)tmp_hum);
        } else {
            LOG_WARNING("NO TEMP: %d", ret);
        }
        vTaskDelay(pdMS_TO_TICKS(DHT_READ_DELAY_MS));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void relay_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 2);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("relay task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    t_relay hot_relay = {0};
    hot_relay.conf.pin = RELAY_HOT_GPIO;
    hot_relay.conf.min_on_sec = 30;
    hot_relay.conf.min_off_sec = 30;
    init_relay(&hot_relay);

    t_relay cool_relay = {0};
    cool_relay.conf.pin = RELAY_COLD_GPIO;
    cool_relay.conf.min_on_sec = 1 * 60;
    cool_relay.conf.min_off_sec = 5 * 60;
    init_relay(&cool_relay);

    t_relay fan_relay = {0};
    fan_relay.conf.pin = RELAY_FAN_GPIO;
    fan_relay.conf.min_on_sec = 10;
    fan_relay.conf.min_off_sec = 10;
    init_relay(&fan_relay);

    init_photor_and_internal_temp(PHOTOR_GPIO);

    vTaskDelay(pdMS_TO_TICKS(3000));  // in case of reboot loop

    char *state2str[] = {"WAIT", "COOL", "HEAT"};

    while (!TEMP_OK(shared__current_temp)) {
        LOG_DEBUG("Waiting for temp...");
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    while (42) {
        ctrl_temp(&hot_relay, &cool_relay);
        LOG_INFO("Relay: %s", state2str[shared__state]);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

        float light_level = read_photor(PHOTOR_ADC_CHANNEL);
        LOG_DEBUG("PHOTOR: %.2f%%", light_level); /* DEBUG */
        if (light_level > LIGHT_LEVEL_TRIGGER) {
            switch_relay(&fan_relay, false); /* DEBUG */
        } else {
            switch_relay(&fan_relay, true); /* DEBUG */
        }
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void menu_task(void *data) {
    (void)data;
    // menu will read / write to flash,
    // so we'll keep all flash access to core 1 for safety
    vTaskCoreAffinitySet(NULL, 1);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("menu task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    init_oled(SCREEN_SCL_GPIO,
                SCREEN_SDA_GPIO,
                SCREEN_SPEED);

    init_menu(BUTTON_UP_GPIO,
              BUTTON_DOWN_GPIO,
              BUTTON_LEFT_GPIO,
              BUTTON_RIGHT_GPIO,
              BUTTON_OK_GPIO);


    while (42) {
        menu_refresh();  // TODO: would make more sense to call from temp thread
        LOG_INFO("Goal: %d°C", shared__goal_temp);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void wifi_task(void *data) {
    (void)data;
    // the http server might need to edit config, so flash access is required,
    // -> core 1
    vTaskCoreAffinitySet(NULL, 1);

    if (cyw43_arch_init()) {
        panic("Wi-Fi init failed");
    }
    led(true);

    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("wifi task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    cyw43_arch_enable_sta_mode();
    if (cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                           CYW43_AUTH_WPA2_AES_PSK,
                                           WIFI_CONNECT_TIMEOUT_MS)) {
        panic("Wi-Fi connect failed");
    }

    /* const char host[] = HOST__RMRF; */
    /* const uint8_t tls_cert[] = TLS_ROOT_CERT__RMRF; */
    const char host[] = HOST__BREW;
    const uint8_t tls_cert[] = TLS_ROOT_CERT__BREW;
    t_http_client_conf conf = {
        .host=host,
        .tls_cert=tls_cert,
        .tls_len=sizeof(tls_cert)
    };

    while (42) {
        vTaskDelay(pdMS_TO_TICKS(HTTP_REQUEST_DELAY_MS));
        http_request(&conf, "/stream/" BREW_KEY,
                     "Content-Type: application/json" EOL,
                     "{\"name\": \"bob\", \"temp\": 22.2}");
/* {"name": "Test3000", "temp": 22.2, "ambient": 27.7, "temp_target": 22, "temp_unit": "C", "hysteresis": 42, "heat_state": "heating", "comment": "pouet"} */
    }

    cyw43_arch_deinit(); // never

    // Do not let a task procedure return
    vTaskDelete(NULL);
}



static void led_task(void *data) {
    (void)data;
    // same core as the wifi task, probably not needed
    // -> core 1
    vTaskCoreAffinitySet(NULL, 1);

    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("led task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    vTaskDelay(pdMS_TO_TICKS(2000)); // wait for init in wifi_task

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
    restore_hot_range();
    restore_cool_range();

    // dht is time sensitive, so the thermo task should be highest priority
    BaseType_t thermo_task_status = xTaskCreate(thermo_task, "thermo_task",
                                                configMINIMAL_STACK_SIZE, NULL,
                                                5, NULL);
    BaseType_t relay_task_status = xTaskCreate(relay_task, "relay_task",
                                               configMINIMAL_STACK_SIZE, NULL,
                                               4, NULL);
    BaseType_t menu_task_status = xTaskCreate(menu_task, "menu_task",
                                              configMINIMAL_STACK_SIZE, NULL,
                                              3, NULL);
    BaseType_t wifi_task_status = xTaskCreate(wifi_task, "wifi_task",
                                              configMINIMAL_STACK_SIZE, NULL,
                                              2, NULL);
    BaseType_t led_task_status = xTaskCreate(led_task, "led_task",
                                              configMINIMAL_STACK_SIZE, NULL,
                                              1, NULL);

    if (thermo_task_status == pdPASS
        && relay_task_status == pdPASS
        && menu_task_status == pdPASS
        && led_task_status == pdPASS
        && wifi_task_status == pdPASS) {

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

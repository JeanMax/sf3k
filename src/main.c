#include <FreeRTOS.h>
#include <hardware/watchdog.h>
#include <task.h>
#include <tusb.h>
#include <pico/cyw43_arch.h>

#include "PinConfig.h"
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
#include "wifi/wifi.h"
#include "wifi/http_client/http_client.h"
#include "wifi/udp_server/udp_server.h"

#define REFRESH_DELAY_MS 1000

#ifdef NDEBUG
# define WATCHDOG_STOP_ON_DEBUG 0
#else
# define WATCHDOG_STOP_ON_DEBUG 1
#endif
#define WATCHDOG_DELAY_MS 1000

#define HTTP_REQUEST_DELAY_MS (15 * 60 * 1000)

//TODO: move these
volatile float shared__current_temp = -42;
volatile int shared__goal_temp = 0;
volatile e_state shared__state = WAIT;
volatile float shared__hot_range = HYSTE_DEFAULT_HOT_RANGE;
volatile float shared__cool_range = HYSTE_DEFAULT_COOL_RANGE;


//TODO: add a sleep.c, move wifi wait to wifi.c, move usb wait to log.c

#define WIFI_CONNECT_TIMEOUT_MS 30000

static int wait_for_wifi(int timeout_ms) {
    uint sleep_inc = 100;
    for (int i = 0; i < timeout_ms; i += sleep_inc) {
        if (is_wifi_connected()) {
            return 0;
        }
        vTaskDelay(pdMS_TO_TICKS(sleep_inc));
    }
    return -1;
}


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



////////////////////////////////////////////////////////////////////////////////



static void thermo_task(void *data) {
    (void)data;
    vTaskCoreAffinitySet(NULL, 2);
    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("thermo task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

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
            LOG_INFO("Temp: %.1f°C - Mean: %.2f°C", tmp_temp, shared__current_temp);
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
        if (is_udp_asking_pause()) {
            switch_relay(&hot_relay, false);
            switch_relay(&cool_relay, false);
        } else {
            ctrl_temp(&hot_relay, &cool_relay);
        }
        LOG_INFO("Relay: %s", state2str[shared__state]);
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));

        float light_level = read_photor(PHOTOR_ADC_CHANNEL);
        LOG_DEBUG("PHOTOR: %.1f%%", light_level); /* DEBUG */
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

//TODO: move to http_client.c ?
#define CONTENT_BUF_SIZE 512
static char *json_encode()
{
    static char json_content[CONTENT_BUF_SIZE];

    snprintf(json_content, CONTENT_BUF_SIZE,
             "{"
             "\"name\": \"SF3K\", "
             "\"temp\": %.1f, "
             "\"ambient\": %.1f, "
             "\"temp_unit\": \"C\", "
             "\"comment\": \"goal=%d, hot_range=%.1f, cool_range=%.1f, state=%s\""
             "}",
             shared__current_temp,
             read_onboard_temperature(INTERNAL_TEMP_ADC_CHANNEL),
             shared__goal_temp,
             shared__hot_range,
             shared__cool_range,
             shared__state == WAIT ? "off" :
                 (shared__state == HEAT ? "heating" : "cooling"));
    return json_content;
}

// deco udp if non-NULL, connect wifi + udp, and wait for LINK_UP
static struct udp_pcb *reconnect(struct udp_pcb *udp) {
    LOG_INFO("trying to reconnect wifi");
    if (udp) {
        udp_server_deinit(udp);
    }
    if (wifi_connect()) {
        panic("Wi-Fi connect failed");
    }
    LOG_INFO("Wi-Fi connected!");
    udp = udp_server_init();
    if (!udp) {
        panic("UDP init failed");
    }
    LOG_INFO("UDP up!");

    LOG_INFO("waiting for LINK_UP...");
    uint sleep_inc = 100;
    for (int i = 0; i < WIFI_CONNECT_TIMEOUT_MS * 2; i += sleep_inc) {
        int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (wifi_status == CYW43_LINK_UP) {
            LOG_INFO("LINK_UP!");
            break;
        }
        vTaskDelay(pdMS_TO_TICKS(sleep_inc));
    }

    return udp;
}

static void wifi_task(void *data) {
    (void)data;
    // the http server might need to edit config, so flash access is required,
    // -> core 1
    vTaskCoreAffinitySet(NULL, 1);

    if (wifi_init()) {
        panic("Wi-Fi init failed");
    }

    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    LOG_INFO("wifi task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    struct udp_pcb *udp = reconnect(NULL);

    while (!TEMP_OK(shared__current_temp)) {
        LOG_DEBUG("Waiting for temp...");
        vTaskDelay(pdMS_TO_TICKS(REFRESH_DELAY_MS));
    }

    while (42) {
        //TODO: log time
        int wifi_status = cyw43_tcpip_link_status(&cyw43_state, CYW43_ITF_STA);
        if (wifi_status != CYW43_LINK_UP) {
            udp = reconnect(udp);
            vTaskDelay(pdMS_TO_TICKS(5000)); // polite sleep
            continue;
        }

        LOG_INFO("Sending http request...");
        int ret = http_request("log.brewersfriend.com",
                               "/stream/" BREW_KEY,
                               "Content-Type: application/json" EOL,
                               json_encode());
        if (!ret) {
            LOG_INFO("request ok!");
        } else {
            if (ret > 0) {  // httpc_result_t;
                LOG_ERROR("request failed: %d (httpc_result_t: ignore)", ret);
            } else {  // err_t
                LOG_ERROR("request failed: %d (err_t: reconnect)", ret);
                udp = reconnect(udp);
            }
        }

        vTaskDelay(pdMS_TO_TICKS(HTTP_REQUEST_DELAY_MS));
    }

    wifi_deinit(); // never

    // Do not let a task procedure return
    vTaskDelete(NULL);
}


static void led_task(void *data) {
    (void)data;
    // same core as the wifi task, probably not needed
    // -> core 1
    vTaskCoreAffinitySet(NULL, 1);

    wait_for_usb(USB_DEFAULT_TIMEOUT_MS);
    if (watchdog_enable_caused_reboot()) {
        LOG_ERROR("Rebooted by Watchdog!");
    }
    LOG_INFO("led task started (core: %d - aff: %lu)",
             portGET_CORE_ID(), vTaskCoreAffinityGet(NULL));

    wait_for_wifi(WIFI_CONNECT_TIMEOUT_MS * 2);
    LOG_DEBUG("led init done");

    watchdog_enable(WATCHDOG_DELAY_MS, WATCHDOG_STOP_ON_DEBUG);

    bool led_state = false;

    while (42) {
        led(led_state);
        led_state ^= 1;
        vTaskDelay(pdMS_TO_TICKS(WATCHDOG_DELAY_MS / 2));
        watchdog_update();
    }

    // Do not let a task procedure return
    vTaskDelete(NULL);
}



////////////////////////////////////////////////////////////////////////////////



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

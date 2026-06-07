#include "wifi.h"


volatile bool g_wifi_connected = false;


int wifi_init()
{
    int ret = cyw43_arch_init();
    if (ret) {
        return ret;
    }
    cyw43_arch_enable_sta_mode();
    return 0;
}

void wifi_deinit()
{
    led(false);
    g_wifi_connected = false;
    cyw43_arch_deinit();
}

int wifi_connect()
{
    int ret = cyw43_arch_wifi_connect_timeout_ms(WIFI_SSID, WIFI_PASSWORD,
                                                 CYW43_AUTH_WPA2_AES_PSK,
                                                 WIFI_CONNECT_TIMEOUT_MS);
    if (ret) {
        return ret;
    }
    g_wifi_connected = true;
    led(true);
    return 0;
}

bool is_wifi_connected() {
    return g_wifi_connected;
}

void led(bool state) {
    if (g_wifi_connected) {
        cyw43_arch_gpio_put(CYW43_WL_GPIO_LED_PIN, state);
    }
}

#ifndef _WIFI_H
#define _WIFI_H

#include <stdbool.h>
#include <pico/cyw43_arch.h>

#define WIFI_CONNECT_TIMEOUT_MS 30000


/** @brief  Init the pico wifi.
 *
 * Required for led support.
 *
 * @return 0 if success
 */
int wifi_init(void);

/** @brief  De-init the pico wifi.
 */
void wifi_deinit(void);

/** @brief  Init the pico led.
 *
 * Connect to the wifi network specified with WIFI_SSID / WIFI_PASSWORD
 *
 * @return 0 if success
 */
int wifi_connect(void);

/** @brief  Check if the wifi is connected.
 *
 * More like 'was it connected?'
 *
 * @return true if connected
 */
bool is_wifi_connected(void);

/** @brief  Switch led state.
 *
 * @param state  turn the led on or off
 */
void led(bool state);

#endif

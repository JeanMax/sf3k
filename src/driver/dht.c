// Copyright (c) 2014 Adafruit Industries
// Author: Tony DiCola

// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:

// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.

// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#include "dht.h"
#include "utils/log.h"          /* DEBUG */

#include <hardware/gpio.h>
#include <FreeRTOS.h>
#include <task.h>


int dht_read(float *temperature, float *humidity, uint8_t dht_pin) {
    *temperature = 0.0f;
    *humidity = 0.0f;

    // Store the count that each DHT bit pulse is low and high.
    // Make sure array is initialized to start at zero.
    uint32_t pulseCounts[DHT_PULSES*2] = {0};

    gpio_set_dir(dht_pin, GPIO_OUT);

    // Set pin high for ~500 milliseconds.
    gpio_put(dht_pin, true);
    vTaskDelay(pdMS_TO_TICKS(500));

    // The next calls are timing critical and care should be taken
    // to ensure no unnecssary work is done below.
    /* taskENTER_CRITICAL(); */

    // Set pin low for ~20 milliseconds.
    gpio_put(dht_pin, false);
    vTaskDelay(pdMS_TO_TICKS(20));

    gpio_set_dir(dht_pin, GPIO_IN);
    // Need a very short delay before reading pins or else value is sometimes still low.
    for (volatile int i = 0; i < 500; ++i) {
    }

    // Wait for DHT to pull pin low.
    uint32_t count = 0;
    while (gpio_get(dht_pin)) {
        if (++count >= DHT_MAXCOUNT) {
            // Timeout waiting for response.
            LOG_ERROR("DHT timeout waiting for pin low"); /* DEBUG */
            /* taskEXIT_CRITICAL(); */
            return DHT_ERROR_TIMEOUT;
        }
    }

    // Record pulse widths for the expected result bits.
    for (int i=0; i < DHT_PULSES*2; i+=2) {
        // Count how long pin is low and store in pulseCounts[i]
        while (!gpio_get(dht_pin)) {
            if (++pulseCounts[i] >= DHT_MAXCOUNT) {
                // Timeout waiting for response.
                LOG_ERROR("DHT timeout waiting for pulse high"); /* DEBUG */
                /* taskEXIT_CRITICAL(); */
                return DHT_ERROR_TIMEOUT;
            }
        }
        // Count how long pin is high and store in pulseCounts[i+1]
        while (gpio_get(dht_pin)) {
            if (++pulseCounts[i+1] >= DHT_MAXCOUNT) {
                // Timeout waiting for response.
                LOG_ERROR("DHT timeout waiting for pulse low"); /* DEBUG */
                /* taskEXIT_CRITICAL(); */
                return DHT_ERROR_TIMEOUT;
            }
        }
    }

    // Done with timing critical code, now interpret the results.
    /* taskEXIT_CRITICAL(); */

    // Compute the average low pulse width to use as a 50 microsecond reference threshold.
    // Ignore the first two readings because they are a constant 80 microsecond pulse.
    uint32_t threshold = 0;
    for (int i=2; i < DHT_PULSES*2; i+=2) {
        threshold += pulseCounts[i];
    }
    threshold /= DHT_PULSES-1;

    // Interpret each high pulse as a 0 or 1 by comparing it to the 50us reference.
    // If the count is less than 50us it must be a ~28us 0 pulse, and if it's higher
    // then it must be a ~70us 1 pulse.
    uint8_t data[5] = {0};
    for (int i=3; i < DHT_PULSES*2; i+=2) {
        int index = (i-3)/16;
        data[index] <<= 1;
        if (pulseCounts[i] >= threshold) {
            // One bit for long pulse.
            data[index] |= 1;
        }
        // Else zero bit for short pulse.
    }

    // Verify checksum of received data.
    if (data[4] == ((data[0] + data[1] + data[2] + data[3]) & 0xFF)) {
        // Calculate humidity and temp for DHT22 sensor.
        *humidity = (float)(data[0] * 256 + data[1]) / 10.0f;
        *temperature = (float)((data[2] & 0x7F) * 256 + data[3]) / 10.0f;
        if (data[2] & 0x80) {
            *temperature *= -1.0f;
        }
        if (TEMP_OK(*temperature) && HYGRO_OK(*humidity)) {
            return DHT_SUCCESS;
        }
        LOG_ERROR("DHT invalid"); /* DEBUG */
        LOG_DEBUG("DHT Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                  data[0], data[1], data[2], data[3], data[4]); /* DEBUG */
        return DHT_ERROR_INVALID;
    }
    else {
        LOG_ERROR("DHT checksum"); /* DEBUG */
        LOG_DEBUG("DHT Data: 0x%x 0x%x 0x%x 0x%x 0x%x\n",
                  data[0], data[1], data[2], data[3], data[4]); /* DEBUG */
        return DHT_ERROR_CHECKSUM;
    }
}


int init_dht(uint8_t dht_pin) {
    gpio_init(dht_pin);
    return 0;
}

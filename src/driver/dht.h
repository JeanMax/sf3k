#ifndef _DHT_H
#define _DHT_H

#include <stdint.h>


/* DHT22 specs:
                            humidity              temperature
Operating range             0-100%RH              -40~80Celsius
Accuracy                    +-2%RH(Max +-5%RH)    <+-0.5Celsius
Resolution or sensitivity   0.1%RH                0.1Celsius
Repeatability               +-1%RH                +-0.2Celsius
Humidity hysteresis         +-0.3%RH
Long-term Stability         +-0.5%RH/year
Sensing period                        Average: 2s
*/


// This is the only processor specific magic value, the maximum amount of time to
// spin in a loop before bailing out and considering the read a timeout.  This should
// be a high value, but if you're running on a much faster platform than a Raspberry
// Pi or Beaglebone Black then it might need to be increased.
#define DHT_MAXCOUNT 32000

// Number of bit pulses to expect from the DHT.  Note that this is 41 because
// the first pulse is a constant 50 microsecond pulse, with 40 pulses to represent
// the data afterwards.
#define DHT_PULSES 41

// Define errors and return values.
#define DHT_ERROR_TIMEOUT -1
#define DHT_ERROR_CHECKSUM -2
#define DHT_ERROR_INVALID -3
#define DHT_ERROR_GPIO -4
#define DHT_SUCCESS 0

// Data validator
#define TEMP_OK(t)  ((t) > -20 && (t) < 60)
#define HYGRO_OK(h) ((h) > 30 && (h) <= 100)

#define DHT_READ_DELAY_MS 2500

/** @brief  Read temperature and humidity from the dht22.
 *
 * Maximum one read every 2 seconds. The protocol is really time sensible,
 * so you should try to run that with maxium priority.
 *
 * @return 0 if success
 */
int dht_read(float *temperature, float *humidity, uint8_t dht_pin);


/** @brief  Init the dht22 sensor.
 *
 * @return 0 if success
 */
int init_dht(uint8_t dht_pin);

#endif

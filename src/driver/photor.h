#ifndef _PHOTOR_H
#define _PHOTOR_H

#include <stdint.h>

#define LIGHT_LEVEL_TRIGGER 50


/** @brief  Init the adc components: photoresistor and internal temp sensor.
 *
 * @param photor_pin  the pin used for the photoresistor
 */
void init_photor_and_internal_temp(uint8_t photor_pin);


/** @brief  Read value from the photoresistor.
 *
 * @param adc_channel  the adc channel to read
 *
 * @return a float value TODO
 */
float read_photor(uint8_t adc_channel);


/** @brief  Read value from the internal temp sensor.
 *
 * @param adc_channel  the adc channel to read
 *
 * @return the temperature (°C) as a float
 */
float read_onboard_temperature(uint8_t adc_channel);

#endif

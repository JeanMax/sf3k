#include "photor.h"

#include "hardware/adc.h"

static inline float raw_to_float(uint16_t raw) {
    /* 12-bit conversion, assume max value == ADC_VREF == 3.3 V */
    const float conversionFactor = 3.3f / (1 << 12);
    return (float)raw * conversionFactor;
}


float read_onboard_temperature(uint8_t adc_channel) {
    if (adc_get_selected_input() != adc_channel) {
        adc_select_input(adc_channel);
    }
    float adc = raw_to_float(adc_read());
    float temp = 27.0f - (adc - 0.706f) / 0.001721f;
    return temp;
}


float read_photor(uint8_t adc_channel) {
    if (adc_get_selected_input() != adc_channel) {
        adc_select_input(adc_channel);
    }
    float adc = raw_to_float(adc_read());
    float ratio = adc / 3.3f * 100.f;
    return 100.f - ratio;
}


void init_photor_and_internal_temp(uint8_t photor_pin) {
    adc_init();
    adc_gpio_init(photor_pin);
    adc_set_temp_sensor_enabled(true);
}

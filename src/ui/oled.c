#include "oled.h"

#include "driver/ss_oled.h"


static SSOLED g_oled = {
    .oled_addr = 0x3c,
    /* .oled_wrap = 0, // ? */
    .oled_flip = 0,
    .oled_type = OLED_128x64,
    /* uint8_t *ucScreen; */
    /* uint8_t iCursorX, iCursorY; */
    /* uint8_t oled_x, oled_y; */
    /* int iScreenOffset; */
    /* .bbi2c.iSCL = scl_pin, */
    /* .bbi2c.iSDA = sda_pin, */
    .bbi2c.picoI2C = i2c_default,
};


int oled_display_string(char *s, int line, int is_big, int is_inverted) {
    return __oledWriteString(&g_oled, 0,
                             0, line * 2,
                             s,
                             is_big ? FONT_16x32 : FONT_16x16,
                             is_inverted, 1);
}


void oled_set_contrast(uint8_t contrast) {
    __oledSetContrast(&g_oled, contrast);
}


void oled_clear() {
    __oledFill(&g_oled, 0, 1);
}


int init_oled(uint8_t scl_pin, uint8_t sda_pin, int32_t speed) {
    g_oled.bbi2c.iSCL = scl_pin;
    g_oled.bbi2c.iSDA = sda_pin;

    int ret = __oledInit(&g_oled, OLED_INVERT, speed);
    if (ret) {
        return ret;
    }

    oled_set_contrast(OLED_DEFAULT_CONTRAST);
    oled_clear();

    return ret;
}

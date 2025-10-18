#include "screen.h"

#include "ss_oled.h"

static uint8_t g_backbuffer[1024] = {0};
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


int display_string(char *s, int line, int is_big, int is_inverted) {
    return __oledWriteString(&g_oled, 0,
                             0, line * 2,
                             s,
                             is_big ? FONT_16x32 : FONT_16x16,
                             is_inverted, 1);
}

void set_screen_contrast(uint8_t contrast) {
    __oledSetContrast(&g_oled, contrast);
}


void clear_screen() {
    __oledFill(&g_oled, 0, 1);
}


int init_screen(uint8_t scl_pin, uint8_t sda_pin) {
    g_oled.bbi2c.iSCL = scl_pin;
    g_oled.bbi2c.iSDA = sda_pin;

    int ret = __oledInit(&g_oled, SCREEN_INVERT, SCREEN_SPEED);
    if (ret == OLED_NOT_FOUND) {
        return -1;
    }
    __oledSetBackBuffer(&g_oled, g_backbuffer);

    set_screen_contrast(SCREEN_DEFAULT_CONTRAST);
    clear_screen();

    return 0;
}

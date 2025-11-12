#include "persist.h"

#include "log.h"
#include "shared.h"

#include <hardware/flash.h>
#include <string.h>


inline static size_t align_page(size_t i) {
    return (i + FLASH_PAGE_SIZE) / FLASH_PAGE_SIZE * FLASH_PAGE_SIZE;
}

static void call_flash_range_program(void *param) {
    const uint8_t *src = (const uint8_t *)((uintptr_t *)param)[0];
    size_t len = align_page(((uintptr_t *)param)[1]);
    size_t flash_addr = ((uintptr_t *)param)[2];
    flash_range_erase(flash_addr, len);  // this is actually important
    flash_range_program(flash_addr, src, len);
}

static void write_to_flash(void *src, size_t len, size_t flash_addr) {
    uintptr_t params[] = { (uintptr_t)src, len, flash_addr };
    int ret = flash_safe_execute(call_flash_range_program, params, 1000);
    if (ret != PICO_OK) {
        LOG_ERROR("Couldn't write to flash: %d", ret);
    }
}

static void read_from_flash(void *dst, size_t len, size_t flash_addr) {
    memcpy(dst, (void *)(XIP_BASE + flash_addr), len);
}


void save_persistent_config() {
    t_persistent_conf conf = {
        .goal_temp = shared__goal_temp,
        .hot_range = shared__hot_range,
        .cool_range = shared__cool_range,
    };
    write_to_flash(&conf, sizeof(conf), BASE_ADDR_PERSISTENT);
}

void restore_goal_temp() {
    read_from_flash((void *)&shared__goal_temp, sizeof(shared__goal_temp),
                   BASE_ADDR_PERSISTENT + GOAL_TEMP_PERSISTENT_OFFSET);
}

void restore_hot_range() {
    read_from_flash((void *)&shared__hot_range, sizeof(shared__hot_range),
                   BASE_ADDR_PERSISTENT + HOT_RANGE_PERSISTENT_OFFSET);
}

void restore_cool_range() {
    read_from_flash((void *)&shared__cool_range, sizeof(shared__cool_range),
                   BASE_ADDR_PERSISTENT + COOL_RANGE_PERSISTENT_OFFSET);
}

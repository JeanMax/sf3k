#ifndef _PERSIST_H
#define _PERSIST_H

#include <pico/flash.h>


extern void *ADDR_PERSISTENT;

#define BASE_ADDR_PERSISTENT ((uint)&ADDR_PERSISTENT - XIP_BASE)


typedef struct persistent_conf  t_persistent_conf;
struct persistent_conf {
    int goal_temp;
    float hot_range;
    float cool_range;
};

#define GOAL_TEMP_PERSISTENT_OFFSET offsetof(t_persistent_conf, goal_temp)
#define HOT_RANGE_PERSISTENT_OFFSET offsetof(t_persistent_conf, hot_range)
#define COOL_RANGE_PERSISTENT_OFFSET offsetof(t_persistent_conf, cool_range)


/** @brief  Save the value of `shared__goal_temp`,  `shared__hot_range`, and
 *  `shared__cool_range` to flash memory.
 *
 * This is done in one batch as a
 * `flash_range_erase` is required before being able to write.
 *
 * @note: Don't call from interrupt
 */
void save_persistent_config(void);


/** @brief  Restore the value of `shared__goal_temp` from flash memory
 *
 * @note: Don't call from interrupt
 */
void restore_goal_temp(void);

/** @brief  Restore the value of `shared__hot_range` from flash memory
 *
 * @note: Don't call from interrupt
 */
void restore_hot_range(void);

/** @brief  Restore the value of `shared__cool_range` from flash memory
 *
 * @note: Don't call from interrupt
 */
void restore_cool_range(void);


#endif

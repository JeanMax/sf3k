#ifndef _PERSIST_H
#define _PERSIST_H

#include <pico/flash.h>


extern void *ADDR_PERSISTENT;

#define BASE_ADDR_PERSISTENT ((uint)&ADDR_PERSISTENT - XIP_BASE)


//TODO: make these more flexible
#define GOAL_TEMP_PERSISTENT_OFFSET 0
#define GOAL_TEMP_PERSISTENT_ADDR (BASE_ADDR_PERSISTENT + GOAL_TEMP_PERSISTENT_OFFSET)



/** @brief  Save the value of `shared__goal_temp` to flash memory
 *
 * @note: Don't call from interrupt
 */
void save_goal_temp(void);


/** @brief  Restore the value of `shared__goal_temp` from flash memory
 *
 * @note: Don't call from interrupt
 */
void restore_goal_temp(void);


#endif

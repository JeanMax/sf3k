#ifndef _PERSIST_H
#define _PERSIST_H

#include <pico/flash.h>


extern void *ADDR_PERSISTENT;

#define GOAL_TEMP_FLASH_ADDR ((uint)&ADDR_PERSISTENT - XIP_BASE)


void save_goal_temp(void);
void restore_goal_temp(void);


#endif

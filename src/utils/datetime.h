#ifndef _DATETIME_H
#define _DATETIME_H

#include <FreeRTOS.h>
#include <task.h>


#define TICK_TO_SEC(t) (pdTICKS_TO_MS(t) / 1000)
#define NOW() (long)TICK_TO_SEC(xTaskGetTickCount())
#define END_OF_TIME INT32_MAX
#define NEVER (INT32_MIN / 2)


#define SLEEP_MS(ms) vTaskDelay(pdMS_TO_TICKS(ms))


/** @brief  Return a formated timestamp
 *
 */
char *get_timestamp_str(void);

#endif

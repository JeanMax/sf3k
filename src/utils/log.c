#include "log.h"

SemaphoreHandle_t g_log_lock;


void init_log_mutex() {
    g_log_lock = xSemaphoreCreateMutex();
}

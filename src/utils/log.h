#ifndef _LOG_H
#define _LOG_H

#include <FreeRTOS.h>
#include <semphr.h>
#include <stdio.h>

extern SemaphoreHandle_t g_log_lock;

#define WITH_LOCK(something) do {                   \
        xSemaphoreTake(g_log_lock, portMAX_DELAY);  \
        something;                                  \
        xSemaphoreGive(g_log_lock);                 \
    } while (0)


#ifdef NO_COLORS
# define CLR_BLACK ""
# define CLR_RED ""
# define CLR_GREEN ""
# define CLR_YELLOW ""
# define CLR_BLUE ""
# define CLR_MAGENTA ""
# define CLR_CYAN ""
# define CLR_WHITE ""
# define CLR_RESET ""
#else
# define CLR_BLACK   "\033[30;01m"
# define CLR_RED     "\033[31;01m"
# define CLR_GREEN   "\033[32;01m"
# define CLR_YELLOW  "\033[33;01m"
# define CLR_BLUE    "\033[34;01m"
# define CLR_MAGENTA "\033[35;01m"
# define CLR_CYAN    "\033[36;01m"
# define CLR_WHITE   "\033[37;01m"
# define CLR_RESET   "\033[0m"
#endif


#ifdef NDEBUG
# define LOG_INFO(str, ...) do {} while (0)
# define LOG_DEBUG(str, ...) do {} while (0)
# define LOG_WARNING(str, ...) do {} while (0)
#else
# define LOG_INFO(str, ...)                                             \
    WITH_LOCK( fprintf(stdout, CLR_BLUE "[INFO]: " CLR_RESET str "\n", ##__VA_ARGS__) )
# define LOG_DEBUG(str, ...)                                            \
    WITH_LOCK( fprintf(stderr, CLR_MAGENTA "[DEBUG]: " CLR_RESET str "\n", ##__VA_ARGS__) )
# define LOG_WARNING(str, ...)                                          \
    WITH_LOCK( fprintf(stderr, CLR_YELLOW "[WARNING]: " CLR_RESET str "\n", ##__VA_ARGS__) )
#endif
#define LOG_ERROR(str, ...)                                             \
    WITH_LOCK( fprintf(stderr, CLR_RED "[ERROR]: " CLR_RESET str "\n", ##__VA_ARGS__) )

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)
#define AT CLR_WHITE __FILE__ ":" TOSTRING(__LINE__) CLR_RESET


/** @brief  Init the mutex used to printf on debug usb
 *
 * @note: This poorly optimized, but since log are disabled
 *        in release, who cares?
 */
void init_log_mutex(void);

#endif

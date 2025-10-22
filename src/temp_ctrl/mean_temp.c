#include "mean_temp.h"

float g_temp_history[MAX_TEMP_HISTORY] = {0};
int g_current_index = 0;
int g_temp_count = 0;

#define CIRCULAR_INDEX(idx)                         \
    (((idx) + MAX_TEMP_HISTORY) % MAX_TEMP_HISTORY)
#define HISTORY_AT(idx)                             \
    g_temp_history[CIRCULAR_INDEX(idx)]



float get_mean_temp() {
    float mean = 0;

    for (int i = 0; i < g_temp_count; i++) {
        mean += (g_temp_history[i] - mean) / (i + 1);
    }

    return mean;
}


void add_temp_to_history(float new_temp) {
    HISTORY_AT(g_current_index) = new_temp;
    g_current_index = CIRCULAR_INDEX(g_current_index + 1);
    if (g_temp_count < MAX_TEMP_HISTORY) {
        g_temp_count++;
    }
}

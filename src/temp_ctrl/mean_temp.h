#ifndef _MEAN_TEMP_H
#define _MEAN_TEMP_H

#define MAX_TEMP_HISTORY 64

float get_mean_temp(void);
void add_temp_to_history(float new_temp);


#endif

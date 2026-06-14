#include "datetime.h"

#include <time.h>


char *get_timestamp_str() {
    time_t now = (time_t)NOW();
    char *now_str = ctime(&now);  // Thu Jan  1 00:01:40 1970
    now_str += 4; // skip day of the week
    now_str[15] = 0; // remove year
    return now_str;
}

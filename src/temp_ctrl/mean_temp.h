#ifndef _MEAN_TEMP_H
#define _MEAN_TEMP_H

#define MAX_TEMP_HISTORY 8

/** @brief  Get the mean temp.
 *
 * This is the mean of the `MAX_TEMP_HISTORY` (or less if not enough data)
 * previous temperatures added with `add_temp_to_history()`.
 *
 * @return the mean temp as a float
 */
float get_mean_temp(void);

/** @brief  Store a temperature in history for future mean calculation.
 *
 * A maximum of `MAX_TEMP_HISTORY` will be kept in history.
 *
 * @param new_temp  a new temperature point to add to the history
 */
void add_temp_to_history(float new_temp);


#endif

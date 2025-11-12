#ifndef _SCREEN_H
#define _SCREEN_H

#define SCREEN_STR_LEN_MAX 8

#define SCREEN_HOT_CHR '\x80'
#define SCREEN_COLD_CHR '\x81'



/** @brief  Display the base screen, which include the current temperature,
 *          relays states, and goal temperature.
 *
 * @return 0 if success
 */
int display_base_screen(void);

/** @brief  Refresh the current temperature part of the base screen.
 *
 * @return 0 if success
 */
int refresh_base_current_temp(void);

/** @brief  Refresh the goal temperature part of the base screen.
 *
 * @return 0 if success
 */
int refresh_base_goal_temp(void);

/** @brief  Refresh the relay state part of the base screen.
 *
 * @return 0 if success
 */
int refresh_base_state(void);


/** @brief  Display a menu composed of 3 string elements.
 *
 * The middle one which is `select`ed, the top one `before`,
 * and the bottom one `after`. (SCREEN_STR_LEN_MAX == 8)
 *
 * @param before  string to display on the top
 * @param select  string to display on the middle (inverted)
 * @param after   string to display on the bottom
 *
 * @return 0 if success
 */
int display_menu_screen(char *before, char *select, char *after);


/** @brief  Display the set goal sub menu.
 *
 * Relay state is still on top, followed by the temperature the user
 * is currently setting, and then a string indicating the current action.
 *
 * @return 0 if success
 */
int display_set_goal_screen(void);

/** @brief  Refresh the temporary goal part of the set goal sub menu.
 *
 * @param tmp_goal  the new temperature goal to display in the set goal menu
 *
 * @return 0 if success
 */
int refresh_set_goal(int tmp_goal);


/** @brief  Display the internal temp submenu
 *
 * Relay state is still on top, followed by the internal temperature of the
 * raspberry pi, and then a string indicating the current action.
 *
 * @return 0 if success
 */
int display_pi_temp_screen(void);

/** @brief  Refresh the temperature part of the internal temp sub menu.
 *
 * @return 0 if success
 */
int refresh_pi_temp();


/** @brief  Display the light level sub menu.
 *
 * Relay state is still on top, followed by the light level given by the
 * photoresistor, and then a string indicating the current action.
 *
 * @return 0 if success
 */
int display_light_level_screen(void);

/** @brief  Refresh the light level part of the sub menu.
 *
 * @return 0 if success
 */
int refresh_light_level();


/** @brief  Display the set hot range sub menu.
 *
 * Relay state is still on top, followed by the temperature range the user
 * is currently setting, and then a string indicating the current action.
 *
 * @return 0 if success
 */
int display_set_hot_range_screen(void);

/** @brief  Refresh the temporary hot_range part of the set hot_range sub menu.
 *
 * @param tmp_hot_range  the new hot_range to display in the set hot_range menu
 *
 * @return 0 if success
 */
int refresh_set_hot_range(float tmp_hot_range);


/** @brief  Display the set cool range sub menu.
 *
 * Relay state is still on top, followed by the temperature range the user
 * is currently setting, and then a string indicating the current action.
 *
 * @return 0 if success
 */
int display_set_cool_range_screen(void);

/** @brief  Refresh the temporary cool_range part of the set cool_range sub menu.
 *
 * @param tmp_cool_range  the new cool_range to display in the set cool_range menu
 *
 * @return 0 if success
 */
int refresh_set_cool_range(float tmp_cool_range);



#endif

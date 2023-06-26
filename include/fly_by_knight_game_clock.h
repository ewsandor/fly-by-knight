/*
  fly_by_knight_game_clock.h
  Fly by Knight - Chess Engine
  Edward Sandor
  June 2023
 
  Logic to manage the game clock
*/

#ifndef __FLY_BY_KNIGHT_GAME_CLOCK_H__
#define __FLY_BY_KNIGHT_GAME_CLOCK_H__

#include "fly_by_knight_types.h"

/**
 * @brief Get time based on realtime clock (not wall-clock)
 * 
 * @param clock_time Output structure for clock_time
 * 
*/
void fbk_get_clock_time(fbk_clock_time_s *clock_time);

/**
 * @brief Initializes game clocks (game clock is not started)
 * 
 * @param fbk    Fly by Knight instance
 * @param config Configuration to apply
 */
void fbk_configure_game_clock(fbk_instance_s * fbk, fbk_game_clock_config_s config);

/**
 * @brief 
 * 
 * @param fbk            Fly by Knight instance
 * @param player         Player's clock to set
 * @param time_remaining Time remaining in milliseconds
 * @param ref_time       Optional reference time.  Will be generated if NULL is passed
 */
void fbk_slam_game_clock(fbk_instance_s * fbk, ftk_color_e player, fbk_time_ms_t time_remaining, const fbk_clock_time_s * ref_time);

/**
 * @brief Get the time spent on the current move in ms
 * 
 * @param fbk 
 * 
 * @return Time in ms
*/
fbk_time_ms_t fbk_get_move_time_ms(fbk_instance_s * fbk);

/**
 * @brief Get the target time the engine should spend thinking on current move (does not consider board position)
 * 
 * @param fbk 
 * 
 * @return Time in ms
*/
fbk_time_ms_t fbk_get_target_move_time_ms(fbk_instance_s * fbk);

#endif /* __FLY_BY_KNIGHT_GAME_CLOCK_H__ */
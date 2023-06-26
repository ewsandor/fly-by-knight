/*
  fly_by_knight_game_clock.c
  Fly by Knight - Chess Engine
  Edward Sandor
  June 2023
 
  Logic to manage the game clock
*/

#include <time.h>

#include "fly_by_knight.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_game_clock.h"

void fbk_configure_game_clock(fbk_instance_s * fbk, fbk_game_clock_config_s config)
{
  fbk_mutex_lock(&fbk->game_lock);

  fbk->game_clock.config = config;

  fbk->game_clock.white_clock.ms_remaining = fbk->game_clock.config.initial_time;
  fbk->game_clock.black_clock.ms_remaining = fbk->game_clock.config.initial_time;

  fbk->game_clock.status = FBK_CLOCK_PAUSED;

  fbk_mutex_unlock(&fbk->game_lock);
}

static inline fbk_time_ms_t timespec_diff(struct timespec *time_a, struct timespec *time_b)
{
  FBK_ASSERT_MSG(time_a != NULL, "Time A is null");
  FBK_ASSERT_MSG(time_b != NULL, "Time A is null");

  const fbk_time_ms_t time_a_ms = time_a->tv_sec*1000 + (time_a->tv_nsec/1000000);
  const fbk_time_ms_t time_b_ms = time_b->tv_sec*1000 + (time_b->tv_nsec/1000000);

  return (time_a_ms - time_b_ms);
}

void fbk_get_clock_time(fbk_clock_time_s *clock_time)
{
  FBK_ASSERT_MSG(clock_time != NULL, "NULL clock_time passed.");
  clock_gettime(CLOCK_MONOTONIC, clock_time);
}

void fbk_slam_game_clock(fbk_instance_s * fbk, ftk_color_e player, fbk_time_ms_t time_remaining, const fbk_clock_time_s * ref_time)
{
  FBK_ASSERT_MSG(fbk != NULL, "Null FBK pointer");
  FBK_ASSERT_MSG((player == FTK_COLOR_WHITE) || (player == FTK_COLOR_BLACK), "Invalid player");

  fbk_player_clock_s *player_clock = ((player == FTK_COLOR_WHITE)?&fbk->game_clock.white_clock:&fbk->game_clock.black_clock);

  player_clock->ms_remaining = time_remaining;

  if(ref_time != NULL)
  {
    player_clock->last_update = *ref_time;
  }
  else
  {
    fbk_get_clock_time(&player_clock->last_update);
  }

  player_clock->set = true;
}

fbk_time_ms_t fbk_get_move_time_ms(fbk_instance_s * fbk)
{
  struct timespec now;
  fbk_get_clock_time(&now);

  return timespec_diff(&now, &fbk->game_clock.last_move_time);
}

#define EXPECTED_MOVES_PER_GAME 50
fbk_time_ms_t fbk_get_target_move_time_ms(fbk_instance_s * fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL FBK instance");

  fbk_time_ms_t target_move_time = fbk->game_clock.max_ms_per_move;
  
  if(FBK_CLOCK_STARTED == fbk->game_clock.status)
  { 
    if(fbk->game.full_move < EXPECTED_MOVES_PER_GAME)
    {
      const fbk_time_ms_t clock_based_target = ((fbk->game_clock.time_at_move_start)*(EXPECTED_MOVES_PER_GAME - fbk->game.full_move - 1))/EXPECTED_MOVES_PER_GAME;

      target_move_time = FBK_MIN(target_move_time, clock_based_target);

    }
    else
    {
      /* TODO: Implement some algorithm when game is in 'overtime' (beyond expected number of moves).  For now, move ASAP */
      target_move_time = 0;
    }
  }

  return target_move_time;
}
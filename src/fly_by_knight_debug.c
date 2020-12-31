/*
 fly_by_knight_debug.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common debug logging and utilities for Fly by Knight
*/

#include "fly_by_knight_debug.h"

//Debug logging level [0(disabled) - 9(maximum)]
fbk_debug_level_t fbk_debug_level = FBK_DEBUG_DISABLED;
  
/**
 * @brief Validates and configures debug logging level
 * 
 * @param new_debug_level 
 */
void fbk_set_debug_level(fbk_debug_level_t new_debug_level)
{
  if(new_debug_level <= FBK_DEBUG_MIN)
  {
    fbk_debug_level = new_debug_level;
  }
}

/**
 * @brief Returns current debug logging level 
 */
fbk_debug_level_t fbk_get_debug_level()
{
  return fbk_debug_level;
}
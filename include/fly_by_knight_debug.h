/*
 fly_by_knight_debug.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common debug logging and utilities for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_DEBUG_H_
#define _FLY_BY_KNIGHT_DEBUG_H_

#include <stdio.h>

#include "fly_by_knight_io.h"

typedef enum
{
  FBK_DEBUG_DISABLED = 0,
  FBK_DEBUG_MAX      = 1,
  FBK_DEBUG_HIGH     = 3,
  FBK_DEBUG_MED      = 5,
  FBK_DEBUG_LOW      = 7,
  FBK_DEBUG_MIN      = 9,
} fbk_debug_level_e;

/**
 * @brief Validates and configures debug logging level
 * 
 * @param new_debug_level 
 */
void fbk_set_debug_level(fbk_debug_level_e new_debug_level);

/**
 * @brief Returns current debug logging level 
 */
fbk_debug_level_e fbk_get_debug_level();

#ifdef __GNUC__
#define FBK_DEBUG_CHECK(level) __builtin_expect((level <= fbk_get_debug_level()), false)
#else
#define FBK_DEBUG_CHECK(level) (level <= fbk_get_debug_level())
#endif

/**
 * @brief Log debug message to STDOUT and logfile if current logging level is set
 * 
 */
#define FBK_DEBUG_MSG(level, msg, ...) if(FBK_DEBUG_CHECK(level)){printf("# [DEBUG] <%u> FBK: " msg "\n", level, ##__VA_ARGS__); FBK_LOG_MSG("# [DEBUG] <%u> FBK: " msg "\n", level, ##__VA_ARGS__);}

#endif //_FLY_BY_KNIGHT_DEBUG_H_
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

#define FBK_DEBUG_MSG(fbk, level, msg, ...) if(level <= (fbk).debug_level){printf("# [DEBUG] <%u> FBK: " msg "\n", level, ##__VA_ARGS__); FBK_LOG_MSG(fbk, "# [DEBUG] <%u> FBK: " msg "\n", level, ##__VA_ARGS__);}

#endif //_FLY_BY_KNIGHT_DEBUG_H_
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
#define FBK_DEBUG_MSG(fbk, msg, ...) if((fbk).debug_mode){printf("# [DEBUG] FBK: " msg "\n", ##__VA_ARGS__);}

#endif //_FLY_BY_KNIGHT_DEBUG_H_
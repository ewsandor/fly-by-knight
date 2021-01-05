/*
 fly_by_knight.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common definitions for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_H_
#define _FLY_BY_KNIGHT_H_

#include "fly_by_knight_types.h"

/* Acknowledge variable is unused */
#define FBK_UNUSED(x) {(void)(x);}

/**
 * @brief Exits Fly by Knight cleanly and return code to calling process
 * 
 * @param return_code 
 */
void fbk_exit(int return_code);

#endif //_FLY_BY_KNIGHT_H_
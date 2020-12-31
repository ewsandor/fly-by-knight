/*
 fly_by_knight_types.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common types for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_TYPES_H_
#define _FLY_BY_KNIGHT_TYPES_H_

#include <stdbool.h>
#include <stdint.h>

//Include Farewell to King library
#include <farewell_to_king_types.h>

/**
 * @brief Chess communication protocol enum
 * 
 */
typedef enum 
{
  FBK_PROTOCOL_UNDEFINED,
  FBK_PROTOCOL_UCI,
  FBK_PROTOCOL_XBOARD
} fbk_protocol_e;

/**
 * @brief Main struction for Fly by Knight
 * 
 */
typedef struct
{
  //Debug mode enabled
  bool debug_mode;

  //Active communication protocol
  fbk_protocol_e protocol;

  //Game state
  ftk_game_s game;

} fbk_instance_s;

#endif //_FLY_BY_KNIGHT_TYPES_H_
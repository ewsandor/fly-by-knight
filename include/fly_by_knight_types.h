/*
 fly_by_knight_types.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
 Common types for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_TYPES_H_
#define _FLY_BY_KNIGHT_TYPES_H_

#include <stdbool.h>
#include <stdint.h>

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

typedef uint8_t xboard_version_t;

typedef enum
{
  FBK_XBOARD_MODE_NORMAL,
  FBK_XBOARD_MODE_EDIT,
} fbk_xboard_mode_e;

typedef struct
{
  /* Active protocol version */
  xboard_version_t version;

  /* Active xboard protocol mode */
  fbk_xboard_mode_e mode;

  /* Edit color */
  ftk_color_e      edit_color;

  /* Color engine should play as */
  ftk_color_e      play_as;

  /* Ponder on opponents turn */
  bool             ponder;

  /* Game result reported */
  bool             result_reported;

} fbk_xboard_data_s;

typedef union
{
  /* Data specific to xboard */
  fbk_xboard_data_s xboard;

} fbk_protocol_data_u;

/**
 * @brief Type for analysis depth
 * 
 */
typedef uint8_t fbk_depth_t;

#define FBK_DEFAULT_MAX_SEARCH_DEPTH 0

typedef enum
{
  FBK_OPPONENT_UNKNOWN,
  FBK_OPPONENT_HUMAN,
  FBK_OPPONENT_COMPUTER,
} fbk_opponent_type_e;

/**
 * @brief Configures engine behavior
 * 
 */
typedef struct
{
  /* Include radom factor for move decision */
  bool                random;

  /* Maximum analysis depth, 0 for no limit */
  fbk_depth_t         max_search_depth;

  /* Output current analysis details */
  bool                analysis_output;

  /* Current opponent type */
  fbk_opponent_type_e opponent_type;

} fbk_engine_config_s;

/**
 * @brief Main structure for Fly by Knight
 * 
 */
typedef struct
{
  /* Active communication protocol */
  fbk_protocol_e protocol;

  /* Active protocol data */
  fbk_protocol_data_u protocol_data;

  /* Engine configuration */
  fbk_engine_config_s config;

  /* Game state */
  ftk_game_s game;

} fbk_instance_s;

#endif //_FLY_BY_KNIGHT_TYPES_H_
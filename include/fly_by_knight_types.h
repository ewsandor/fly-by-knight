/*
 fly_by_knight_types.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
 Common types for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_TYPES_H_
#define _FLY_BY_KNIGHT_TYPES_H_

#include <pthread.h>

#include <stdbool.h>
#include <stdint.h>
#include <stdint.h>

#include <farewell_to_king_types.h>

/**
 * @brief Mutex to be used throughout Fly by Knight
 * 
 */
typedef pthread_mutex_t fbk_mutex_t;

/**
 * @brief Type for scoring games in millipawns.  + White advantage, - Black advantage
 * 
 */
typedef int_fast32_t fbk_score_t;

/**
 * @brief Count of Move Tree nodes
 * 
 */
typedef uint_fast16_t fbk_move_tree_node_count_t;

/**
 * @brief Move Tree node structure
 * 
 */
typedef struct fbk_move_tree_node_struct fbk_move_tree_node_s;
struct fbk_move_tree_node_struct{

  /* Lock for accessing and modifying node */
  fbk_mutex_t                 lock;

  /* Move represented by this node, invalid if root node*/
  ftk_move_s                  move;

  /* TRUE if this node has been evaluated */
  bool                        evaluated;
  /* Score considering this node alone */
  fbk_score_t                 base_score;

  /* TODO - Add best path info */

  /* Parent node pointer, NULL if root node */
  fbk_move_tree_node_s       *parent;
  /* Number of child nodes, only valid after node is evaluated */ 
  fbk_move_tree_node_count_t  child_count;
  /* Array of 'child_count' child nodes */
  fbk_move_tree_node_s       *child;
};

typedef struct fbk_move_tree_struct fbk_move_tree_s;
struct fbk_move_tree_struct
{
  /* Root node of tree */
  fbk_move_tree_node_s  root;

  /* Current active node of tree */
  fbk_move_tree_node_s *current;
};


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

/**
 * @brief Type for analysis breadth
 * 
 */
typedef uint8_t fbk_breadth_t;

#define FBK_DEFAULT_MAX_SEARCH_BREADTH 0

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

  /* Number of worker threads */
  unsigned int        worker_threads;

} fbk_engine_config_s;

/**
 * @brief Main structure for Fly by Knight
 * 
 */
typedef struct
{
  /* Active communication protocol */
  fbk_protocol_e      protocol;

  /* Active protocol data */
  fbk_protocol_data_u protocol_data;

  /* Engine configuration */
  fbk_engine_config_s config;

  /* Game state */
  ftk_game_s          game;

  /* Move tree for current game */
  fbk_move_tree_s     move_tree;

} fbk_instance_s;

#endif //_FLY_BY_KNIGHT_TYPES_H_
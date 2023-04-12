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
 * @brief Type to indicate thread index through Fly by Knight
*/
typedef unsigned int fbk_thread_index_t;

/**
 * @brief Type for scoring games in millipawns.  + White advantage, - Black advantage
 * 
 */
typedef int_fast32_t fbk_score_t;

/**
 * @brief Type for analysis depth
 * 
 */
typedef uint_fast16_t fbk_depth_t;
#define FBK_DEFAULT_MAX_SEARCH_DEPTH 0
#define FBK_MAX_DEPTH                ((1<<16)-1)

/**
 * @brief Type for analysis breadth
 * 
 */
typedef uint8_t fbk_breadth_t;
#define FBK_DEFAULT_MAX_SEARCH_BREADTH 0

#define FBK_MOVE_TREE_MAX_NODE_COUNT ((1<<8)-1)
/**
 * @brief Count of Move Tree nodes
 * 
 */
typedef uint8_t fbk_move_tree_node_count_t;

typedef struct
{
  /* TRUE if this node has been evaluated */
  bool                       evaluated;
  /* Score considering this node alone */
  fbk_score_t                base_score;
  /* Min and Max child analysis depth */
  fbk_depth_t                min_depth;
  fbk_depth_t                max_depth;
  /* Child node with best analysis */
  fbk_score_t                best_child_score;
  fbk_move_tree_node_count_t best_child_index;

} fbk_move_tree_node_analysis_data_s;

/**
 * @brief Move Tree node structure
 * 
 */
typedef struct fbk_move_tree_node_struct fbk_move_tree_node_s;
struct fbk_move_tree_node_struct{

  /* Lock for accessing and modifying node */
  fbk_mutex_t                         lock;

  /* Move represented by this node, invalid if root node*/
  ftk_move_s                          move;

  /* Analysis data for this node */
  fbk_move_tree_node_analysis_data_s  analysis_data;

  /* Parent node pointer, NULL if root node or compressed */
  fbk_move_tree_node_s               *parent;
  /* Number of child nodes, only valid after node is evaluated */ 
  fbk_move_tree_node_count_t          child_count;
  /* Array of 'child_count' child nodes, NULL if compressed */
  fbk_move_tree_node_s               *child;
  /* Size of compressed child data structure.  0 if not compressed */
  size_t                              child_compressed_size;
  /* Compressed array of child nodes.  NULL if not compressed */
  void                               *child_compressed;
};

typedef struct fbk_move_tree_struct fbk_move_tree_s;
struct fbk_move_tree_struct
{
  /* True if the move tree has been initialized */
  bool                  initialized;

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
  #ifdef FBK_UCI_PROTOCOL_SUPPORT
  FBK_PROTOCOL_UCI,
  #endif
  #ifdef FBK_XBOARD_PROTOCOL_SUPPORT
  FBK_PROTOCOL_XBOARD
  #endif
} fbk_protocol_e;

#ifdef FBK_XBOARD_PROTOCOL_SUPPORT
typedef uint8_t xboard_version_t;

typedef enum
{
  FBK_XBOARD_MODE_NORMAL,
  FBK_XBOARD_MODE_FORCE,
  FBK_XBOARD_MODE_WAITING,
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
#endif

typedef union
{
  char              unused;
  #ifdef FBK_XBOARD_PROTOCOL_SUPPORT
  /* Data specific to xboard */
  fbk_xboard_data_s xboard;
  #endif

} fbk_protocol_data_u;

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
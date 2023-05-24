/*
 fly_by_knight_pick.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move picking and decision making for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_PICK_H_
#define _FLY_BY_KNIGHT_PICK_H_

#include "fly_by_knight_types.h"

typedef void (*fbk_pick_callback_f)          (ftk_game_end_e, ftk_move_s, void *);

typedef struct fbk_picker_best_line_node_struct fbk_picker_best_line_node_s;

struct fbk_picker_best_line_node_struct
{
  /* Current move in best line */
  ftk_move_s                   move;

  /* Next move in best line */
  fbk_picker_best_line_node_s *next_move;

};

typedef struct
{
  /* Reference game for best line */
  ftk_game_s                          game;
  /* Score information */
  fbk_move_tree_node_analysis_data_s  analysis_data;
  /* Number of child node from best line root */
  fbk_move_tree_node_count_t          child_count;
  /* Current move search time in ms */
  fbk_time_ms_t                       search_time;
  /* Number of nodes searched */
  fbk_node_count_t                    searched_node_count;
  /* Table base hits - currently unused (value 0) */
  unsigned long int                   tbhits;

  /* First move in best line */
  fbk_picker_best_line_node_s        *first_move;

} fbk_picker_best_line_s;

typedef void (*fbk_pick_best_line_callback_f)(const fbk_picker_best_line_s *, void *);

/* Picker client config */
typedef struct
{
  /* Color to pick for */
  ftk_color_e                   play_as;

  /* Callback to be called when a move is picked and commited */
  fbk_pick_callback_f           pick_callback;
  void *                        pick_user_data_ptr;

  /* Callback to be called when the best line is updated without committing */
  fbk_pick_best_line_callback_f best_line_callback;
  void *                        best_line_user_data_ptr;

} fbk_picker_client_config_s;

/**
 * @brief Types of picker logic triggers
*/
typedef enum 
{
  FBK_PICKER_TRIGGER_INVALID,
  FBK_PICKER_TRIGGER_PICKER_STARTED,
  FBK_PICKER_TRIGGER_MOVE_COMMITTED,
  FBK_PICKER_TRIGGER_FORCED,
  FBK_PICKER_TRIGGER_TIMED,
  FBK_PICKER_TRIGGER_JOB_ENDED,
} fbk_picker_trigger_e;

typedef union
{
  const fbk_move_tree_node_s * job_node;
} fbk_picker_trigger_data_u;

/**
 * @brief Picker logic trigger data
*/
typedef struct
{
  fbk_picker_trigger_e      type;

  fbk_picker_trigger_data_u data;

} fbk_picker_trigger_s;


/**
 * @brief Logic to initialize the picking logic
 * 
 * @param fbk Fly by Knight instance
 * 
 * @return true if successful
*/
bool fbk_init_picker(fbk_instance_s *fbk);

/**
 * @brief Start the picker logic
 * 
 * @param pick_client_config Client specific picker configuration
 * 
*/
void fbk_start_picker(const fbk_picker_client_config_s *pick_client_config);

/**
 * @brief Stop the picker logic and block until stop
 * 
*/
void fbk_stop_picker();

/**
 * @brief Trigger the picker to reevaluate best move
 * 
 * @param trigger Trigger data
*/
void fbk_trigger_picker(const fbk_picker_trigger_s * trigger);

#endif //_FLY_BY_KNIGHT_PICK_H_
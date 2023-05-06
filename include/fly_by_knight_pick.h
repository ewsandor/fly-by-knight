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
  /* Score information */
  /* placeholder */

  /* First move in best line */
  fbk_picker_best_line_node_s *first_move;

} fbk_picker_best_line_s;

typedef void (*fbk_pick_best_line_callback_f)(fbk_picker_best_line_s, void *);

/* Picker client config */
typedef struct
{
  /* Color to pick for */
  ftk_color_e            play_as;

  /* Callback to be called when a move is picked and commited */
  fbk_pick_callback_f    pick_callback;
  void *                 pick_user_data_ptr;

  /* Callback to be called when the best line is updated without committing */
  fbk_picker_best_line_s best_line_callback;
  void *                 best_line_user_data_ptr;

} fbk_picker_client_config_s;

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



#endif //_FLY_BY_KNIGHT_PICK_H_
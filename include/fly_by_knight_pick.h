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

typedef void (*fbk_pick_callback_f)(ftk_game_end_e, ftk_move_s, void *);

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
*/
void fbk_start_picker(fbk_pick_callback_f, void * user_data_ptr);

/**
 * @brief Stop the picker logic and block until stop
 * 
*/
void fbk_stop_picker();



#endif //_FLY_BY_KNIGHT_PICK_H_
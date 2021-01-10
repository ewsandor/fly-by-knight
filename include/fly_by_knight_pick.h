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

/**
 * @brief Returns the best move based on the current game
 * 
 * @param fbk         Fly by Knight instance
 * @return ftk_move_s Best move
 */
ftk_move_s fbk_get_best_move(fbk_instance_s *fbk);

#endif //_FLY_BY_KNIGHT_PICK_H_
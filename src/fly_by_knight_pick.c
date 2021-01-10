/*
 fly_by_knight_pick.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move picking and decision making for Fly by Knight
*/

#include <farewell_to_king.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_pick.h"

/**
 * @brief Returns the random legal move based on the current game
 * 
 * @param fbk         Fly by Knight instance
 * @return ftk_move_s Random legal move
 */
ftk_move_s fbk_get_random_move(fbk_instance_s *fbk)
{
  ftk_move_list_s move_list;
  ftk_move_s random_move;

  ftk_get_move_list(&fbk->game, &move_list);

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Found %lu legal moves", move_list.count);

  if(move_list.count > 0)
  {
    random_move = move_list.move[rand() % move_list.count];
  }
  else
  {
    ftk_invalidate_move(&random_move);
  }

  ftk_delete_move_list(&move_list);

  return random_move;
}

/**
 * @brief Returns the best move based on the current game
 * 
 * @param fbk         Fly by Knight instance
 * @return ftk_move_s Best move
 */
ftk_move_s fbk_get_best_move(fbk_instance_s *fbk)
{
  unsigned int i;
  ftk_game_s      game_temp = fbk->game;
  ftk_move_s      best_move;
  fbk_score_t     move_score, best_score;
  ftk_move_list_s move_list;

  ftk_invalidate_move(&best_move);

  best_score = (FTK_COLOR_WHITE == fbk->game.turn)?FBK_SCORE_BLACK_MAX:FBK_SCORE_WHITE_MAX;

  ftk_get_move_list(&fbk->game, &move_list);
  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Found %lu legal moves", move_list.count);

  for(i = 0; i < move_list.count; i++)
  {
    FBK_ASSERT_MSG(FTK_MOVE_VALID(move_list.move[i]), "Invalid move in move_list");

    ftk_move_forward(&game_temp, &move_list.move[i]);
    move_score = fbk_score_game(&game_temp);
    ftk_move_backward(&game_temp, &move_list.move[i]);

    if(((FTK_COLOR_WHITE == fbk->game.turn) && (move_score > best_score)) ||
       ((FTK_COLOR_BLACK == fbk->game.turn) && (move_score < best_score)))
    {
      best_score = move_score;
      best_move  = move_list.move[i];
      FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Best move so far score %ld (%u->%u)", best_score, best_move.source, best_move.target);
    }
  }
    
  ftk_delete_move_list(&move_list);

  return best_move;
}
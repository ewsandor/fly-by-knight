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
  ftk_move_s      best_move;
  fbk_score_t     move_score, best_score;

  ftk_invalidate_move(&best_move);

  best_score = (FTK_COLOR_WHITE == fbk->game.turn)?FBK_SCORE_BLACK_MAX:FBK_SCORE_WHITE_MAX;

  FBK_ASSERT_MSG(NULL != fbk->move_tree.current, "Current move tree node NULL");

  fbk_evaluate_move_tree_node_children(fbk->move_tree.current, fbk->game);

  for(i = 0; i < fbk->move_tree.current->child_count; i++)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&fbk->move_tree.current->child[i].lock), "Failed to lock node mutex");
    FBK_ASSERT_MSG(fbk->move_tree.current->child[i].analysis_data.evaluated, "Node %u not evaluated", i);
    move_score = fbk->move_tree.current->child[i].analysis_data.base_score;
    FBK_ASSERT_MSG(true == fbk_mutex_unlock(&fbk->move_tree.current->child[i].lock), "Failed to unlock node mutex");

    if(((FTK_COLOR_WHITE == fbk->game.turn) && (move_score > best_score)) ||
       ((FTK_COLOR_BLACK == fbk->game.turn) && (move_score < best_score)))
    {
      best_score = move_score;
      best_move  = fbk->move_tree.current->child[i].move;
      FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Best move so far score %ld (%u->%u)", best_score, best_move.source, best_move.target);
    }
  }
    
  return best_move;
}
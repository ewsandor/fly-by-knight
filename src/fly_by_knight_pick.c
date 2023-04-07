/*
 fly_by_knight_pick.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move picking and decision making for Fly by Knight
*/

#include <unistd.h>

#include <farewell_to_king.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_analysis_worker.h"
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

  fbk_move_tree_node_s** sorted_nodes = malloc(fbk->move_tree.current->child_count * sizeof(fbk_move_tree_node_s*));
  fbk_mutex_lock(&fbk->move_tree.current->lock);
  fbk_sort_child_nodes(fbk->move_tree.current, sorted_nodes);
  fbk_mutex_unlock(&fbk->move_tree.current->lock);
  free(sorted_nodes);

  for(i = 0; i < fbk->move_tree.current->child_count; i++)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&fbk->move_tree.current->child[i].lock), "Failed to lock node mutex");
    FBK_ASSERT_MSG(fbk->move_tree.current->child[i].analysis_data.evaluated, "Node %u not evaluated", i);
    if(fbk->move_tree.current->child[i].analysis_data.best_child_index < fbk->move_tree.current->child_count)
    {
      move_score = fbk->move_tree.current->child[i].analysis_data.best_child_score;
    }
    else
    {
      move_score = fbk->move_tree.current->child[i].analysis_data.base_score;
    }
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

typedef struct
{
  fbk_instance_s      *fbk;

  fbk_mutex_t          lock;
  pthread_t            picker_thread;
  pthread_cond_t       pick_started_cond;

  bool                 picker_active;
  ftk_color_e          play_as;

  fbk_pick_callback_f  pick_cb;
  void                *pick_cb_user_data_ptr;

} fbk_pick_data_s;
fbk_pick_data_s pick_data = {0};

void * picker_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL arg passed.");

  fbk_pick_data_s * pick_data = (fbk_pick_data_s *) arg;

  while(1)
  {
    fbk_mutex_lock(&pick_data->lock);
    while(pick_data->picker_active == false)
    {
      pthread_cond_wait(&pick_data->pick_started_cond, &pick_data->lock);
    }
    
    ftk_game_end_e game_result = ftk_check_for_game_end(&pick_data->fbk->game);
    ftk_move_s     move        = {0};

    bool report      = false;
    bool commit_move = false;

    if(FTK_END_NOT_OVER != game_result)
    {
      fbk_stop_analysis(true);
      report = true;
    }
    else if(FTK_END_NOT_OVER == game_result && (pick_data->fbk->game.turn == pick_data->play_as))
    {
      move = fbk_get_best_move(pick_data->fbk);

      /* Commit move */
      commit_move = true;
    }

    if((commit_move || report) && (pick_data->pick_cb != NULL))
    {
      pick_data->pick_cb(game_result, move, pick_data->pick_cb_user_data_ptr);
    }

    if(commit_move)
    {
      fbk_stop_analysis(true);
      FBK_ASSERT_MSG(true == fbk_commit_move(pick_data->fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
    }

    fbk_mutex_unlock(&pick_data->lock);

    sleep(1);
  }
}

bool fbk_init_picker(fbk_instance_s *fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk instance passed.");

  fbk_mutex_init(&pick_data.lock);
  pthread_cond_init(&pick_data.pick_started_cond, NULL);

  pick_data.fbk                   = fbk;

  pick_data.picker_active         = false;
  pick_data.play_as               = FTK_COLOR_NONE;

  pick_data.pick_cb               = NULL;
  pick_data.pick_cb_user_data_ptr = NULL;

  FBK_ASSERT_MSG(0 == pthread_create(&pick_data.picker_thread , NULL, picker_thread_f, &pick_data), "Failed to start picker thread.");

  return true;
}

void fbk_start_picker(ftk_color_e play_as, fbk_pick_callback_f callback, void * user_data_ptr)
{
  fbk_mutex_lock(&pick_data.lock);
  pick_data.pick_cb               = callback;
  pick_data.pick_cb_user_data_ptr = user_data_ptr;
  pick_data.picker_active         = true;
  pick_data.play_as               = play_as;
  pthread_cond_broadcast(&pick_data.pick_started_cond);
  fbk_mutex_unlock(&pick_data.lock);
}

void fbk_stop_picker()
{
  fbk_mutex_lock(&pick_data.lock);
  pick_data.pick_cb               = NULL;
  pick_data.pick_cb_user_data_ptr = NULL;
  pick_data.picker_active         = true;
  pick_data.play_as               = FTK_COLOR_NONE;
  fbk_mutex_unlock(&pick_data.lock);
}
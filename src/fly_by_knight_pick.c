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
#include "fly_by_knight_move_tree.h"
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
  FBK_ASSERT_MSG(NULL != fbk->move_tree.current, "Current move tree node NULL");

  fbk_move_tree_node_s** sorted_nodes = malloc(fbk->move_tree.current->child_count * sizeof(fbk_move_tree_node_s*));
  fbk_mutex_lock(&fbk->move_tree.current->lock);

  bool decompressed = fbk_decompress_move_tree_node(fbk->move_tree.current, true);

  ftk_move_s best_move = {0};
  ftk_invalidate_move(&best_move);

  if(fbk->move_tree.current->child_count > 0)
  {
    if(fbk_sort_child_nodes(fbk->move_tree.current, sorted_nodes))
    {
      best_move = sorted_nodes[fbk->move_tree.current->child_count-1]->move;
    }
    else
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Failed to sort child nodes.  Returning invalid move.");
    }
  }

  if(decompressed)
  {
    fbk_compress_move_tree_node(fbk->move_tree.current, true);
  }

  fbk_mutex_unlock(&fbk->move_tree.current->lock);
  free(sorted_nodes);
   
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
    
    bool analysis_restart_needed = fbk_stop_analysis(false);

    FBK_DEBUG_MSG(FBK_DEBUG_MED, "Considering best move for picking logic.");

    ftk_game_end_e game_result = ftk_check_for_game_end(&pick_data->fbk->game);
    ftk_move_s     move        = {0};

    bool report      = false;
    bool commit_move = false;

    if(FTK_END_NOT_OVER != game_result)
    {
      fbk_stop_analysis(true);
      analysis_restart_needed = false;
      report = true;
    }
    else if(FTK_END_NOT_OVER == game_result && (pick_data->fbk->game.turn == pick_data->play_as))
    {
      move = fbk_get_best_move(pick_data->fbk);
      
      if(FTK_MOVE_VALID(move))
      {
        /* Commit move */
        commit_move = true;
      }
      else
      {
        FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Could not find best move, not selecting any move yet.");
      }
    }
  
    if((commit_move || report) && (pick_data->pick_cb != NULL))
    {
      if(commit_move)
      {
        fbk_stop_analysis(true);
        analysis_restart_needed = false;
        FBK_ASSERT_MSG(true == fbk_commit_move(pick_data->fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
      }
      pick_data->pick_cb(game_result, move, pick_data->pick_cb_user_data_ptr);
    }

    if(analysis_restart_needed)
    {
      fbk_start_analysis(&pick_data->fbk->game, pick_data->fbk->move_tree.current);
    }

    fbk_mutex_unlock(&pick_data->lock);

    sleep(5);
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
  pick_data.picker_active         = false;
  pick_data.play_as               = FTK_COLOR_NONE;
  fbk_mutex_unlock(&pick_data.lock);
}
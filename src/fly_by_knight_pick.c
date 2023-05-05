/*
 fly_by_knight_pick.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move picking and decision making for Fly by Knight
*/

#include <unistd.h>

#include <farewell_to_king.h>

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
 * @param node Locked and decompressed node to get best child
 * @return Best move tree node
 */
static inline fbk_move_tree_node_s *fbk_get_best_move(const fbk_move_tree_node_s * node)
{
  FBK_ASSERT_MSG(NULL != node, "Current move tree node NULL");

  fbk_move_tree_node_s** sorted_nodes = malloc(node->child_count * sizeof(fbk_move_tree_node_s*));

  fbk_move_tree_node_s * best_node = NULL;

  if(node->child_count > 0)
  {
    if(fbk_sort_child_nodes(node, sorted_nodes))
    {
      best_node = sorted_nodes[node->child_count-1];
    }
    else
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Failed to sort child nodes.  Returning invalid move.");
    }
  }

  free(sorted_nodes);
   
  return best_node;
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
    
    if(pick_data->picker_active == true)
    {
      /* Make sure picker is still active */
      FBK_DEBUG_MSG(FBK_DEBUG_MED, "Considering best move for picking logic.");

      ftk_game_end_e game_result = ftk_check_for_game_end(&pick_data->fbk->game);
      ftk_move_s     move        = {0};
      ftk_invalidate_move(&move);

      bool report      = false;
      bool commit_move = false;

      if(FTK_END_NOT_OVER != game_result)
      {
        fbk_stop_analysis(true);
        report = true;
      }
      else if(FTK_END_NOT_OVER == game_result && (pick_data->fbk->game.turn == pick_data->play_as))
      {
        fbk_mutex_lock(&pick_data->fbk->move_tree.current->lock);
        bool decompressed = fbk_decompress_move_tree_node(pick_data->fbk->move_tree.current, true);
        fbk_move_tree_node_s *best_node = fbk_get_best_move(pick_data->fbk->move_tree.current);
        if(best_node != NULL)
        {
          fbk_mutex_lock(&best_node->lock);
          move = best_node->move;
          fbk_mutex_unlock(&best_node->lock);
        }
        if(decompressed)
        {
          fbk_compress_move_tree_node(pick_data->fbk->move_tree.current, true);
        }
        fbk_mutex_unlock(&pick_data->fbk->move_tree.current->lock);

 
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
          FBK_ASSERT_MSG(true == fbk_commit_move(pick_data->fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
        }
        pick_data->pick_cb(game_result, move, pick_data->pick_cb_user_data_ptr);
      }
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
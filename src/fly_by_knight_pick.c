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

  fbk_pick_best_line_callback_f best_line_callback;
  void *                        best_line_user_data_ptr;

} fbk_pick_data_s;
fbk_pick_data_s pick_data = {0};

static fbk_picker_best_line_node_s *build_best_line(fbk_move_tree_node_s *move_tree_node)
{
  FBK_ASSERT_MSG(move_tree_node != NULL, "Null move_tree_node");

  fbk_picker_best_line_node_s *new_node = malloc(sizeof(fbk_picker_best_line_node_s));
  FBK_ASSERT_MSG(new_node != NULL, "Failed to allocate new best-line node");

  fbk_mutex_lock(&move_tree_node->lock);
  bool decompressed = fbk_decompress_move_tree_node(move_tree_node, true);
 
  new_node->move = move_tree_node->move;
  if(move_tree_node->analysis_data.evaluated && (move_tree_node->analysis_data.best_child_index < move_tree_node->child_count))
  {
    new_node->next_move = build_best_line(&move_tree_node->child[move_tree_node->analysis_data.best_child_index]);
  }
  else
  {
    new_node->next_move = NULL;
  }

  if(decompressed)
  {
    fbk_compress_move_tree_node(move_tree_node, true);
  }
  fbk_mutex_unlock(&move_tree_node->lock);

  return new_node;
}

static inline void delete_best_line(fbk_picker_best_line_node_s *best_line)
{
  if(best_line->next_move != NULL)
  {
    delete_best_line(best_line->next_move);
  }
  free(best_line);
}

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

      if(FTK_END_NOT_OVER == game_result)
      {
        fbk_mutex_lock(&pick_data->fbk->move_tree.current->lock);
        bool decompressed = fbk_decompress_move_tree_node(pick_data->fbk->move_tree.current, true);
        fbk_move_tree_node_s *best_node = fbk_get_best_move(pick_data->fbk->move_tree.current);
        if(best_node != NULL)
        {
          fbk_picker_best_line_s best_line = {0};

          fbk_mutex_lock(&best_node->lock);
          move = best_node->move;
          best_line.analysis_data = best_node->analysis_data;
          fbk_mutex_unlock(&best_node->lock);

          if((pick_data->fbk->config.thinking_output == true) && (pick_data->best_line_callback != NULL))
          {
            best_line.search_time = fbk_get_move_time_ms(pick_data->fbk);
            best_line.searched_node_count = get_analyzed_nodes();
            best_line.first_move = build_best_line(best_node);
            pick_data->best_line_callback(&best_line, pick_data->best_line_user_data_ptr);
            delete_best_line(best_line.first_move);
          }
        }
        if(decompressed)
        {
          fbk_compress_move_tree_node(pick_data->fbk->move_tree.current, true);
        }
        fbk_mutex_unlock(&pick_data->fbk->move_tree.current->lock);

 
        if(FTK_MOVE_VALID(move))
        {
          if(pick_data->fbk->game.turn == pick_data->play_as)
          {
            /* Commit move */
            commit_move = true;
          }
        }
        else
        {
          FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Could not find best move, not selecting any move yet.");
        }
      }
      else
      {
        fbk_stop_analysis(true);
        report = true;
      }
 
    
      if((commit_move || report) && (pick_data->pick_cb != NULL))
      {
        if(commit_move)
        {
          fbk_stop_analysis(true);
          FBK_ASSERT_MSG(true == fbk_commit_move(pick_data->fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
          game_result = ftk_check_for_game_end(&pick_data->fbk->game);
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

void fbk_start_picker(const fbk_picker_client_config_s *pick_client_config)
{
  fbk_mutex_lock(&pick_data.lock);
  pick_data.pick_cb                 = pick_client_config->pick_callback;
  pick_data.pick_cb_user_data_ptr   = pick_client_config->pick_user_data_ptr;
  pick_data.best_line_callback      = pick_client_config->best_line_callback;
  pick_data.best_line_user_data_ptr = pick_client_config->best_line_user_data_ptr;
  pick_data.picker_active           = true;
  pick_data.play_as                 = pick_client_config->play_as;
  pthread_cond_broadcast(&pick_data.pick_started_cond);
  fbk_mutex_unlock(&pick_data.lock);
}

void fbk_stop_picker()
{
  fbk_mutex_lock(&pick_data.lock);
  pick_data.pick_cb                 = NULL;
  pick_data.pick_cb_user_data_ptr   = NULL;
  pick_data.best_line_callback      = NULL;
  pick_data.best_line_user_data_ptr = NULL;
  pick_data.picker_active           = false;
  pick_data.play_as                 = FTK_COLOR_NONE;
  fbk_mutex_unlock(&pick_data.lock);
}
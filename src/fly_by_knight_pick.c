/*
 fly_by_knight_pick.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move picking and decision making for Fly by Knight
*/

#include <string.h>
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

typedef struct fbk_picker_trigger_node_struct fbk_picker_trigger_node_s;
struct fbk_picker_trigger_node_struct
{
  fbk_picker_trigger_s       trigger;
  fbk_picker_trigger_node_s *next_trigger;
};

typedef struct
{
  fbk_mutex_t                lock;
  pthread_cond_t             new_trigger_condition;

  unsigned int               trigger_count;
  fbk_picker_trigger_node_s *first_trigger;
  fbk_picker_trigger_node_s *last_trigger;
  
} fbk_picker_trigger_queue_s;

typedef struct
{
  fbk_instance_s                *fbk;

  fbk_mutex_t                    lock;
  pthread_t                      picker_thread;

  fbk_picker_trigger_queue_s     trigger_queue;

  bool                           picker_active;
  ftk_color_e                    play_as;

  fbk_pick_callback_f            pick_cb;
  void                          *pick_cb_user_data_ptr;

  fbk_pick_best_line_callback_f  best_line_callback;
  void *                         best_line_user_data_ptr;

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

/**
 * @brief Logic to add job to back of job queue.  Assumes caller has lock.
 * @param queue       queue to append
 * @param new_trigger new trigger to append
*/
static void push_trigger_to_trigger_queue(fbk_picker_trigger_queue_s * queue, const fbk_picker_trigger_s * new_trigger)
{

  FBK_ASSERT_MSG(queue       != NULL, "NULL queue passed.");
  FBK_ASSERT_MSG(new_trigger != NULL, "NULL trigger passed.");

  /* Build new node */
  fbk_picker_trigger_node_s *new_trigger_node = (fbk_picker_trigger_node_s *) malloc(sizeof(fbk_picker_trigger_node_s));
  FBK_ASSERT_MSG(new_trigger_node != NULL, "Malloc failed");
  new_trigger_node->next_trigger = NULL;
  new_trigger_node->trigger      = *new_trigger;

  if(queue->trigger_count > 0)
  {
    /* Add to linked list */
    queue->last_trigger->next_trigger = new_trigger_node;
    queue->last_trigger               = new_trigger_node;
  }
  else
  {
    /* Init root job */
    queue->first_trigger = new_trigger_node;
    queue->last_trigger  = new_trigger_node;
  }

  queue->trigger_count++;
  FBK_ASSERT_MSG(0 == pthread_cond_signal(&queue->new_trigger_condition), "Failed to signal new trigger is available.");
}

/**
 * @brief Returns the next trigger in the trigger queue or NULL if no triggers are available.  Assumes caller has lock.
 * @param queue Job queue to pop from.
 * 
 * @return Next trigger in queue or 'invalid' trigger if queue is empty
*/
static fbk_picker_trigger_s pop_trigger_from_trigger_queue(fbk_picker_trigger_queue_s * queue)
{
  fbk_picker_trigger_s ret_val = {0};

  FBK_ASSERT_MSG(queue != NULL, "NULL job queue passed.");

  if(queue->trigger_count > 0)
  {
    ret_val = queue->first_trigger->trigger;
    queue->first_trigger = queue->first_trigger->next_trigger;

    queue->trigger_count--;

    if(queue->trigger_count == 0)
    {
      FBK_ASSERT_MSG(NULL == queue->first_trigger, "All jobs claimed, but last job is not pointing to NULL");
      queue->last_trigger = NULL;
    }
  }

  return ret_val;
}

void * picker_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL arg passed.");

  fbk_pick_data_s * pick_data = (fbk_pick_data_s *) arg;

  bool force_move = false;

  while(1)
  {
    fbk_mutex_lock(&pick_data->trigger_queue.lock);

    while(pick_data->trigger_queue.trigger_count == 0)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Waiting for picker trigger.");
      pthread_cond_wait(&pick_data->trigger_queue.new_trigger_condition, &pick_data->trigger_queue.lock);
    }
    fbk_picker_trigger_s trigger = pop_trigger_from_trigger_queue(&pick_data->trigger_queue);
    FBK_ASSERT_MSG(trigger.type != FBK_PICKER_TRIGGER_INVALID, "Invalid picker tirgger.");
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Popped picker trigger %u.", trigger.type);

    fbk_mutex_unlock(&pick_data->trigger_queue.lock);

    if(FBK_PICKER_TRIGGER_MOVE_COMMITTED == trigger.type)
    {
      force_move = false;
    }
    else if(FBK_PICKER_TRIGGER_FORCED == trigger.type)
    {
      force_move = true;
    }

    fbk_mutex_lock(&pick_data->lock);
   
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
        bool post = false;
        fbk_picker_best_line_s best_line = {0};

        fbk_mutex_lock(&pick_data->fbk->move_tree.current->lock);
        bool decompressed = fbk_decompress_move_tree_node(pick_data->fbk->move_tree.current, true);
        fbk_move_tree_node_s *best_node = fbk_get_best_move(pick_data->fbk->move_tree.current);
        if(best_node != NULL)
        {
          fbk_mutex_lock(&best_node->lock);
          move = best_node->move;
          best_line.analysis_data = best_node->analysis_data;
          fbk_mutex_unlock(&best_node->lock);

          post = true;
          best_line.search_time = fbk_get_move_time_ms(pick_data->fbk);
          best_line.searched_node_count = get_analyzed_nodes();

          if((FBK_PICKER_TRIGGER_JOB_ENDED == trigger.type) && (trigger.data.job_node != best_node))
          {
            post = false;
          }

        }
        if(FTK_MOVE_VALID(move) &&
           (pick_data->fbk->game.turn == pick_data->play_as))
        { 
          /* Move is valid and for current turn, check criteria to commit */
          if(force_move)
          {
            commit_move = true;
          }
          else if( FTK_END_DEFINITIVE(best_line.analysis_data.result)        ||  FTK_END_DEFINITIVE(best_line.analysis_data.best_child_result) ||
                  (FTK_END_DRAW_STALEMATE == best_line.analysis_data.result) || (FTK_END_DRAW_STALEMATE == best_line.analysis_data.best_child_result))
          {
            /* Definitive result */
            commit_move = true;
          }
          else if(best_line.searched_node_count > 1000000)
          {
            /* Analyzed 1,000,000 moves*/
            commit_move = true;
          }
          else if((best_line.searched_node_count > 500000) &&
                  (best_line.analysis_data.best_child_depth >= 5))
          {
            /* Analyzed 500,000 moves and best line depth 5 */
            commit_move = true;
          }
          else if((best_line.searched_node_count > 250000) &&
                  (best_line.analysis_data.best_child_depth >= 6))
          {
            /* Analyzed 250,000 moves and best line depth 6 */
            commit_move = true;
          }
        }
        else
        {
          FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Could not find best move, not selecting any move yet.");
        }

        if( commit_move ||
            (post && (pick_data->fbk->config.thinking_output) && (pick_data->best_line_callback != NULL)) )
        {
          best_line.first_move = build_best_line(best_node);
          pick_data->best_line_callback(&best_line, pick_data->best_line_user_data_ptr);
          delete_best_line(best_line.first_move);
        }

        if(decompressed)
        {
          fbk_compress_move_tree_node(pick_data->fbk->move_tree.current, true);
        }
        fbk_mutex_unlock(&pick_data->fbk->move_tree.current->lock);
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
  }
}

bool fbk_init_picker(fbk_instance_s *fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk instance passed.");

  memset(&pick_data, 0, sizeof(fbk_pick_data_s));

  fbk_mutex_init(&pick_data.lock);

  pick_data.fbk                   = fbk;
  pick_data.picker_active         = false;

  fbk_mutex_init(&pick_data.trigger_queue.lock);
  pthread_cond_init(&pick_data.trigger_queue.new_trigger_condition, NULL);

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
  fbk_mutex_unlock(&pick_data.lock);
  const fbk_picker_trigger_s trigger = 
  {
    .type = FBK_PICKER_TRIGGER_PICKER_STARTED,
  };
  fbk_trigger_picker(&trigger);
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

void fbk_trigger_picker(const fbk_picker_trigger_s * trigger)
{
  FBK_ASSERT_MSG(trigger != NULL, "Null trigger passed");

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Queuing picker trigger %u.", trigger->type);

  fbk_mutex_lock(&pick_data.trigger_queue.lock);
  push_trigger_to_trigger_queue(&pick_data.trigger_queue, trigger);
  fbk_mutex_unlock(&pick_data.trigger_queue.lock);
}
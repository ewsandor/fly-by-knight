/*
 fly_by_knight_analysis.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Gama analysis for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_analysis_types.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_move_tree.h"

fbk_analysis_data_s fbk_analysis_data = {0};

fbk_score_t fbk_score_potential_capture_value(ftk_type_e piece_type)
{
  fbk_score_t score = 0;

  switch (piece_type)
  {
    case FTK_TYPE_PAWN:
    {
      score = FBK_SCORE_CAPTURE_PAWN;
      break;
    }
    case FTK_TYPE_KNIGHT:
    {
      score = FBK_SCORE_CAPTURE_KNIGHT;
      break;
    }
    case FTK_TYPE_BISHOP:
    {
      score = FBK_SCORE_CAPTURE_BISHOP;
      break;
    }
    case FTK_TYPE_ROOK:
    {
      score = FBK_SCORE_CAPTURE_ROOK;
      break;
    }
    case FTK_TYPE_QUEEN:
    {
      score = FBK_SCORE_CAPTURE_QUEEN;
      break;
    }
    case FTK_TYPE_KING:
    {
      score = FBK_SCORE_CAPTURE_KING;
      break;
    }
    default:
    {
      break;
    }
  }

  return score;
}

fbk_score_t fbk_score_potential_loss_value(ftk_type_e piece_type)
{
  fbk_score_t score = 0;

  switch (piece_type)
  {
    case FTK_TYPE_PAWN:
    {
      score = FBK_SCORE_LOSS_PAWN;
      break;
    }
    case FTK_TYPE_KNIGHT:
    {
      score = FBK_SCORE_LOSS_KNIGHT;
      break;
    }
    case FTK_TYPE_BISHOP:
    {
      score = FBK_SCORE_LOSS_BISHOP;
      break;
    }
    case FTK_TYPE_ROOK:
    {
      score = FBK_SCORE_LOSS_ROOK;
      break;
    }
    case FTK_TYPE_QUEEN:
    {
      score = FBK_SCORE_LOSS_QUEEN;
      break;
    }
    case FTK_TYPE_KING:
    {
      score = FBK_SCORE_LOSS_KING;
      break;
    }
    default:
    {
      break;
    }
  }

  return score;
}

fbk_score_t fbk_score_potential_capture(ftk_square_s square, ftk_color_e turn)
{
  fbk_score_t score;

  if(turn == square.color)
  {
    score = fbk_score_potential_loss_value(square.type);
  }
  else
  {
    score = fbk_score_potential_capture_value(square.type);
  }

  if(FTK_COLOR_WHITE == square.color)
  {
    /* If capture square is white, black is moving */
    score *= -1;
  }

  return score;
}

/**
 * @brief Score game position for white or black advantage
 * 
 * @param game      to analyze
 * @return fbk_score_t 
 */
fbk_score_t fbk_score_game(const ftk_game_s * game)
{
  fbk_score_t score = 0;
  unsigned int i;
  int         advantage;

  int legal_move_count;
  ftk_board_mask_t capture_mask;
  ftk_square_e   capture_square;
  unsigned int white_king_not_moved  = 0;
  unsigned int white_rooks_not_moved = 0;
  unsigned int black_king_not_moved  = 0;
  unsigned int black_rooks_not_moved = 0;

  for(i = 0; i < FTK_STD_BOARD_SIZE; i++)
  {
    advantage = (FTK_COLOR_WHITE == game->board.square[i].color)?1:-1;
    legal_move_count = ftk_get_num_bits_set(game->board.move_mask[i]);

    capture_mask = game->board.move_mask[i] & game->board.board_mask;
    while(capture_mask)
    { 
      capture_square = ftk_get_first_set_bit_idx(capture_mask);
      FTK_CLEAR_BIT(capture_mask, capture_square);
      score += fbk_score_potential_capture(game->board.square[capture_square], game->turn);
    }

    switch(game->board.square[i].type)
    {
      case FTK_TYPE_PAWN:
      {
        score += advantage*(FBK_SCORE_PAWN + (legal_move_count*FBK_SCORE_PAWN_MOVE));
        break;
      }
      case FTK_TYPE_KNIGHT:
      {
        score += advantage*(FBK_SCORE_KNIGHT + (legal_move_count*FBK_SCORE_KNIGHT_MOVE));
        break;
      }
      case FTK_TYPE_BISHOP:
      {
        score += advantage*(FBK_SCORE_BISHOP + (legal_move_count*FBK_SCORE_BISHOP_MOVE));
        break;
      }
      case FTK_TYPE_ROOK:
      {
        score += advantage*(FBK_SCORE_ROOK + (legal_move_count*FBK_SCORE_ROOK_MOVE));

        if(FTK_MOVED_NOT_MOVED == game->board.square[i].moved)
        {
          if(FTK_COLOR_WHITE == game->board.square[i].color)
          {
            white_rooks_not_moved++;
          }
          else
          {
            black_rooks_not_moved++;
          }
        }

        break;
      }
      case FTK_TYPE_QUEEN:
      {
        score += advantage*(FBK_SCORE_QUEEN + (legal_move_count*FBK_SCORE_QUEEN_MOVE));
        break;
      }
      case FTK_TYPE_KING:
      {
        score += advantage*(FBK_SCORE_KING + (legal_move_count*FBK_SCORE_KING_MOVE));

        if(FTK_MOVED_NOT_MOVED == game->board.square[i].moved)
        {
          if(FTK_COLOR_WHITE == game->board.square[i].color)
          {
            white_king_not_moved = true;
          }
          else
          {
            black_king_not_moved = true;
          }
        }

        break;
      }
      default:
      {
        break;
      }
    }
  }

  /* Weight the ability to castle still */
  if(white_king_not_moved)
  {
    score += FBK_SCORE_CAN_CASTLE * white_rooks_not_moved;
  }
  if(black_king_not_moved)
  {
    score -= FBK_SCORE_CAN_CASTLE * black_rooks_not_moved;
  }
   
  return score;
}

/**
 * @brief Evaluates all children of node
 * 
 * @param node 
 * @param game 
 */
void fbk_evaluate_move_tree_node_children(fbk_move_tree_node_s * node, ftk_game_s game)
{
  unsigned int i;
  FBK_ASSERT_MSG(node != NULL, "NULL node passed");

  fbk_evaluate_move_tree_node(node, &game);

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  FBK_ASSERT_MSG(true == node->evaluated, "Failed to evaluate node");
  for(i = 0; i < node->child_count; i++)
  {
    FBK_ASSERT_MSG(fbk_apply_move_tree_node(&node->child[i], &game), "Failed to apply node %u", i);
    fbk_evaluate_move_tree_node(&node->child[i], &game);
    FBK_ASSERT_MSG(fbk_undo_move_tree_node(&node->child[i], &game),  "Failed to undo node %u", i);
  }
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
}

/**
 * @brief Initialized Fly by Knight analysis data
 * 
 * @param fbk 
 * @return true if successful
 */
bool fbk_init_analysis_data(fbk_instance_s *fbk)
{
  bool ret_val = true;

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Initializing analysis data");

  FBK_ASSERT_MSG(fbk != NULL, "NULL FBK instance passed");

  if(false == fbk_analysis_data.initialized)
  {
    memset(&fbk_analysis_data, 0, sizeof(fbk_analysis_data_s));

    fbk_analysis_data.fbk = fbk;

    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.analysis_stats.lock), "Failed to initialize analysis stats");

    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.job_queue.lock), "Failed to initialize analysis stats");
    FBK_ASSERT_MSG(0 == pthread_cond_init(&fbk_analysis_data.job_queue.new_job_available, NULL), "Failed to initialize analysis stats");
  }
  else
  {
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Analysis data already initialized");
    ret_val = false;
  }

  return ret_val;
}

static void worker_thread_cleanup(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Cleaning up worker thread %u.", worker_thread_data->thread_id);
}

static void worker_thread_job_queue_cleanup(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u releasing job queue mutex for cancellation.", worker_thread_data->thread_id);
  fbk_mutex_unlock(&worker_thread_data->job_queue->lock);
}

static void * worker_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;

  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u started.", worker_thread_data->thread_id);

  pthread_cleanup_push(worker_thread_cleanup, worker_thread_data);

  while(1)
  {
    fbk_mutex_lock(&worker_thread_data->job_queue->lock);
    pthread_cleanup_push(worker_thread_job_queue_cleanup, worker_thread_data);
    while(worker_thread_data->job_queue->next_job == NULL)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u waiting for job.", worker_thread_data->thread_id);
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&worker_thread_data->job_queue->new_job_available, &worker_thread_data->job_queue->lock),
        "Failed Waiting for new data available condition.");
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(0);

    /* Claim job and set pthread cancellation cleanup callback */
    fbk_mutex_unlock(&worker_thread_data->job_queue->lock);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    /* Process job */
    
    /* Report job done or cancelled and remove pthread cancellation callback */
  }

  pthread_cleanup_pop(0);
}
/**
 * @brief Updates the configured number of worker threads
 * 
 * @param count new number of worker threads to allow
*/
void fbk_update_worker_thread_count(unsigned int count)
{
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Configuring %u worker threads.", count);

  if(count < 1)
  {
    FBK_ERROR_MSG("At least 1 worker thread is required but %u is requested.  Falling back to 1 worker thread", count);
    fbk_analysis_data.fbk->config.worker_threads = 1;
  }
  else
  {
    fbk_analysis_data.fbk->config.worker_threads = count;
  }

  if(fbk_analysis_data.fbk->config.worker_threads > fbk_analysis_data.worker_thread_count)
  {
    /* Spawn worker threads */

    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Allocating memory for new worker thread(s).");
    /* Allocate additional worker thread data entries */
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker threads failed.\n", fbk_analysis_data.fbk->config.worker_threads);

    for(unsigned int i = fbk_analysis_data.worker_thread_count; i < fbk_analysis_data.fbk->config.worker_threads; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Creating worker thread %u.", i);
      /* Configure each new entry */
      memset(&fbk_analysis_data.worker_thread_data[i], 0, sizeof(fbk_worker_thread_data_s));
      fbk_analysis_data.worker_thread_data[i].thread_id        = i;
      fbk_analysis_data.worker_thread_data[i].job_queue        = &fbk_analysis_data.job_queue;
      fbk_analysis_data.worker_thread_data[i].analysis_allowed = fbk_analysis_data.analysis_active;
      pthread_create(&fbk_analysis_data.worker_thread_data[i].worker_thread, NULL, worker_thread_f, &fbk_analysis_data.worker_thread_data[i]);
    }
  }
  else if(fbk_analysis_data.fbk->config.worker_threads < fbk_analysis_data.worker_thread_count)
  {
    /* Kill worker threads */

    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Cancelling worker thread %u.", i);
      /* Trigger cancellation for all excess worker threads */
      pthread_cancel(fbk_analysis_data.worker_thread_data[i].worker_thread);
    }
    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      /* Wait for all excess worker threads to finish cancellation */
      FBK_ASSERT_MSG(0 == pthread_join(fbk_analysis_data.worker_thread_data[i].worker_thread, NULL), 
                    "Failed to join worker thread %u.", i);
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u cancelled.", i);
    }
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Releasing memory for excess worker thread(s).");
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker threads failed.\n", fbk_analysis_data.fbk->config.worker_threads);
  }

  fbk_analysis_data.worker_thread_count = fbk_analysis_data.fbk->config.worker_threads;
}
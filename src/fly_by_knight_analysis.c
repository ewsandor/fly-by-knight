/*
 fly_by_knight_analysis.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Core gama analysis for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_move_tree.h"

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

bool fbk_evaluate_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game, bool locked)
{
  bool ret_val = false;

  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  }

  if(false == node->analysis_data.evaluated)
  {
    ftk_update_board_masks(game);

    memset(&node->analysis_data, 0, sizeof(fbk_move_tree_node_analysis_data_s));

    /* Check for game end */
    node->analysis_data.result = ftk_check_for_game_end(game);

    if(FTK_END_NOT_OVER == node->analysis_data.result)
    {
      node->analysis_data.base_score = fbk_score_game(game);

      /* Init child nodes */
      ftk_move_list_s move_list = {0};
      ftk_get_move_list(game, &move_list);
      node->child_count = move_list.count;

      if(node->child_count > 0)
      {
        node->child = malloc(node->child_count*sizeof(fbk_move_tree_node_s));

        for(unsigned int i = 0; i < node->child_count; i++)
        {
          fbk_init_move_tree_node(&node->child[i], node, &move_list.move[i]);
        }
      }

      node->analysis_data.best_child_index = node->child_count;
      node->analysis_data.best_child_score =  (FTK_COLOR_WHITE == game->turn)?FBK_SCORE_BLACK_MAX:FBK_SCORE_WHITE_MAX;

      ftk_delete_move_list(&move_list);
    }

    node->analysis_data.evaluated = true;
    ret_val = true;
  }

  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
  }

  return ret_val;
}
/**
 * @brief Clears evaluation and deletes all child nodes
 * 
 * @param node 
 */
void fbk_unevaluate_move_tree_node(fbk_move_tree_node_s * node)
{
  unsigned int i;


  FBK_ASSERT_MSG(node != NULL, "Null node passed");

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Deleting move_tree_node node %p (%u->%u)", (void*) node, node->move.source, node->move.target);

  fbk_decompress_move_tree_node(node, true);

  for(i = 0; i < node->child_count; i++)
  {
    fbk_delete_move_tree_node(&node->child[i]);
  }

  node->child_count = 0;
  free(node->child);
  node->child = NULL;

  memset(&node->analysis_data, 0, sizeof(fbk_move_tree_node_analysis_data_s));

  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
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

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  bool decompressed = fbk_decompress_move_tree_node(node, true);
  fbk_evaluate_move_tree_node(node, &game, true);
  FBK_ASSERT_MSG(true == node->analysis_data.evaluated, "Failed to evaluate node");
  for(i = 0; i < node->child_count; i++)
  {
    FBK_ASSERT_MSG(fbk_apply_move_tree_node(&node->child[i], &game), "Failed to apply node %u", i);
    fbk_evaluate_move_tree_node(&node->child[i], &game, false);
    FBK_ASSERT_MSG(fbk_undo_move_tree_node(&node->child[i], &game),  "Failed to undo node %u", i);
  }
  if(decompressed)
  {
    fbk_compress_move_tree_node(node, true);
  }
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
}

int fbk_compare_move_tree_nodes(const void *a, const void *b)
{
  int ret_val = 0;

  const fbk_move_tree_node_s *node_a = *((fbk_move_tree_node_s **) a);
  const fbk_move_tree_node_s *node_b = *((fbk_move_tree_node_s **) b);

  FBK_ASSERT_MSG(node_a->analysis_data.evaluated, "Node A not evaluated");
  FBK_ASSERT_MSG(node_b->analysis_data.evaluated, "Node B not evaluated");
  FBK_ASSERT_MSG(node_a->move.turn == node_b->move.turn, "Node turns do not match");

  /* Check for the better node as white */
  if(node_a->analysis_data.result == FTK_END_NOT_OVER)
  {
    fbk_score_t node_a_score = (node_a->analysis_data.best_child_index < node_a->child_count)?
                              (node_a->analysis_data.best_child_score):(node_a->analysis_data.base_score);

    if(node_b->analysis_data.result == FTK_END_NOT_OVER)
    {
      fbk_score_t node_b_score = (node_b->analysis_data.best_child_index < node_b->child_count)?
                                (node_b->analysis_data.best_child_score):(node_b->analysis_data.base_score);

      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.result))
    {
      /* If child depth is even, then node B will checkmate in the current turn's favor.  
         If odd, then node B will checkmate in the other turn's favor. */
      ret_val = ((node_b->analysis_data.best_child_depth % 2) == 0)?-1:1;
    }
    else if(FTK_END_DRAW(node_b->analysis_data.result))
    {
      fbk_score_t node_b_score = 0;
      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else
    {
      FBK_FATAL_MSG("Unhandled node game result");
    }
  }
  else if(FTK_END_DEFINITIVE(node_a->analysis_data.result))
  {
    if( (node_b->analysis_data.result == FTK_END_NOT_OVER) ||
        (FTK_END_DRAW(node_b->analysis_data.result)) )
    {
      /* If child depth is even, then node A will checkmate in the current turn's favor.  
         If odd, then node A will checkmate in the other turn's favor. */
      ret_val = ((node_a->analysis_data.best_child_depth % 2) == 0)?1:-1;
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.result))
    {

    }
    else
    {
      FBK_FATAL_MSG("Unhandled node game result");
    }
  }
  else if(FTK_END_DRAW(node_a->analysis_data.result))
  {
    fbk_score_t node_a_score = 0;

    if(node_b->analysis_data.result == FTK_END_NOT_OVER)
    {
      fbk_score_t node_b_score = (node_b->analysis_data.best_child_index < node_b->child_count)?
                                (node_b->analysis_data.best_child_score):(node_b->analysis_data.base_score);

      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.result))
    {
      /* If child depth is even, then node B will checkmate in the current turn's favor.  
         If odd, then node B will checkmate in the other turn's favor. */
      ret_val = ((node_b->analysis_data.best_child_depth % 2) == 0)?-1:1;
    }
    else if(FTK_END_DRAW(node_b->analysis_data.result))
    {
      /* Both nodes are a draw, choose the longer path as an opponent is more likely to make a mistake */
      ret_val = (node_a->analysis_data.best_child_depth - node_b->analysis_data.best_child_depth);
    }
    else
    {
      FBK_FATAL_MSG("Unhandled node game result");
    }
  }
  else
  {
    FBK_FATAL_MSG("Unhandled node game result");
  }

  return ret_val;
}

bool fbk_sort_child_nodes(fbk_move_tree_node_s * node, fbk_move_tree_node_s* sorted_nodes[])
{
  FBK_ASSERT_MSG(node != NULL,         "NULL node passed");
  FBK_ASSERT_MSG(sorted_nodes != NULL, "NULL sorted_nodes buffer passed");

  bool ret_val = true;
  fbk_move_tree_node_count_t nodes_locked = 0;

  /* Prep child nodes and initialize pointers */
  for(fbk_move_tree_node_count_t i = 0; i < node->child_count; i++)
  {
    sorted_nodes[i] = &node->child[i];
    fbk_mutex_lock(&sorted_nodes[i]->lock);
    nodes_locked++;
    if(false == sorted_nodes[i]->analysis_data.evaluated)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Child node %u is not yet analyzed (%u->%u).  Aborting sort.", i, sorted_nodes[i]->move.source, sorted_nodes[i]->move.target);
      ret_val = false;
      break;
    }
  }

  if(true == ret_val)
  {
    qsort(sorted_nodes, node->child_count, sizeof(fbk_move_tree_node_s*), fbk_compare_move_tree_nodes);
  }

  /* Release pointers */
  for(fbk_move_tree_node_count_t i = 0; (i < node->child_count) && (nodes_locked > 0); i++)
  {
    fbk_mutex_unlock(&sorted_nodes[i]->lock);
    nodes_locked--;
  }

  return ret_val;
}
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
    /* Score position, compound == base as no child nodes evaluated */
    memset(&node->analysis_data, 0, sizeof(fbk_move_tree_node_analysis_data_s));
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
  fbk_decompress_move_tree_node(node, true);

  node->analysis_data.evaluated = false;
  node->analysis_data.base_score = 0;

  if(node->analysis_data.evaluated)
  {
    for(i = 0; i < node->child_count; i++)
    {
      fbk_delete_move_tree_node(&node->child[i]);
    }

    node->child_count = 0;
    free(node->child);
  }

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
  fbk_decompress_move_tree_node(node, true);
  fbk_evaluate_move_tree_node(node, &game, true);
  FBK_ASSERT_MSG(true == node->analysis_data.evaluated, "Failed to evaluate node");
  for(i = 0; i < node->child_count; i++)
  {
    FBK_ASSERT_MSG(fbk_apply_move_tree_node(&node->child[i], &game), "Failed to apply node %u", i);
    fbk_evaluate_move_tree_node(&node->child[i], &game, false);
    FBK_ASSERT_MSG(fbk_undo_move_tree_node(&node->child[i], &game),  "Failed to undo node %u", i);
  }
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
}

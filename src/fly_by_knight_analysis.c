/*
 fly_by_knight_analysis.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Core gama analysis for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_hash.h"
#include "fly_by_knight_move_tree.h"

/* Constant after initialized */
static const struct fbk_analysis_lookup_table_struct
{
  bool initialized;

  fbk_score_t white_pawn_position_score  [FTK_STD_BOARD_SIZE];
  fbk_score_t black_pawn_position_score  [FTK_STD_BOARD_SIZE];
  fbk_score_t white_knight_position_score[FTK_STD_BOARD_SIZE];
  fbk_score_t black_knight_position_score[FTK_STD_BOARD_SIZE];

} *lut = NULL;

bool fbk_init_analysis_lut()
{
  bool ret_val = true;

  if(lut == NULL)
  {
    struct fbk_analysis_lookup_table_struct *new_lut = calloc(1, sizeof(*new_lut));

    for(unsigned int i = 0; i < FTK_STD_BOARD_SIZE; i++)
    {
      switch(i/8)
      {
        case 0:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FIRST_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_EIGHTH_ROW;
          break;
        }
        case 1:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_SECOND_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_SEVENTH_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_SECOND_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_SEVENTH_ROW;
          break;
        }
        case 2:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_THIRD_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_SIXTH_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_THIRD_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_SIXTH_ROW;
          break;
        }
        case 3:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_FOURTH_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_FIFTH_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FOURTH_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FIFTH_ROW;
          break;
        }
        case 4:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_FIFTH_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_FOURTH_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FIFTH_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FOURTH_ROW;
          break;
        }
        case 5:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_SIXTH_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_THIRD_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_SIXTH_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_THIRD_ROW;
          break;
        }
        case 6:
        {
          new_lut->white_pawn_position_score[i] += FBK_SCORE_PAWN_SEVENTH_ROW;
          new_lut->black_pawn_position_score[i] += FBK_SCORE_PAWN_SECOND_ROW;

          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_SEVENTH_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_SECOND_ROW;
          break;
        }
        case 7:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_EIGHTH_ROW;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FIRST_ROW;
          break;
        }
        default:
        {
          FBK_FATAL_MSG("Unexpected position %u", i);
          break;
        }
      }

      switch(i%8)
      {
        case 0:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_A;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_A;
          break;
        }
        case 1:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_B;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_B;
          break;
        }
        case 2:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_C;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_C;
          break;
        }
        case 3:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_D;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_D;
          break;
        }
        case 4:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_E;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_E;
          break;
        }
        case 5:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_F;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_F;
          break;
        }
        case 6:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_G;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_G;
          break;
        }
        case 7:
        {
          new_lut->white_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_H;
          new_lut->black_knight_position_score[i] += FBK_SCORE_KNIGHT_FILE_H;
          break;
        }
        default:
        {
          FBK_FATAL_MSG("Unexpected position %u", i);
          break;
        }
      }
    }


    /* Score pawn center squares */
    new_lut->white_pawn_position_score[FTK_D4] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->white_pawn_position_score[FTK_E4] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->white_pawn_position_score[FTK_D5] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->white_pawn_position_score[FTK_E5] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->black_pawn_position_score[FTK_D4] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->black_pawn_position_score[FTK_E4] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->black_pawn_position_score[FTK_D5] = FBK_SCORE_PAWN_CENTER_SQUARE;
    new_lut->black_pawn_position_score[FTK_E5] = FBK_SCORE_PAWN_CENTER_SQUARE;


    /* Follow Kasparov's 'Knight on F6 is worth a Pawn'*/
    new_lut->white_knight_position_score[FTK_F6] = FBK_SCORE_PAWN;
    new_lut->black_knight_position_score[FTK_F3] = FBK_SCORE_PAWN;

    new_lut->initialized = true;
    lut = new_lut;
  }

  return ret_val;
}

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

  unsigned int white_pawns_on_file[8] = {0};
  unsigned int black_pawns_on_file[8] = {0};

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

        if(game->board.square[i].color == FTK_COLOR_WHITE)
        {
          white_pawns_on_file[i % 8]++;
          score += lut->white_pawn_position_score[i];
        }
        else
        {
          black_pawns_on_file[i % 8]++;
          score -= lut->black_pawn_position_score[i];
        }

        break;
      }
      case FTK_TYPE_KNIGHT:
      {
        score += advantage*(FBK_SCORE_KNIGHT + (legal_move_count*FBK_SCORE_KNIGHT_MOVE));

        if(game->board.square[i].color == FTK_COLOR_WHITE)
        {
          score += lut->white_knight_position_score[i];
        }
        else
        {
          score -= lut->black_knight_position_score[i];
        }

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
        else if ( (i == FTK_G1) && 
                  (game->board.square[FTK_G1].color == FTK_COLOR_WHITE) && 
                  (game->board.square[FTK_F1].type  == FTK_TYPE_ROOK) && 
                  (game->board.square[FTK_F1].color == FTK_COLOR_WHITE))
        {
          /* White is castled kingside */
          score += FBK_SCORE_CASTLED_KINGSIDE;
        }
        else if ( (i == FTK_G8) && 
                  (game->board.square[FTK_G8].color == FTK_COLOR_BLACK) && 
                  (game->board.square[FTK_F8].type  == FTK_TYPE_ROOK) && 
                  (game->board.square[FTK_F8].color == FTK_COLOR_BLACK))
        {
          /* Black is castled kingside */
          score -= FBK_SCORE_CASTLED_KINGSIDE;
        }
        else if ( (i == FTK_C1) && 
                  (game->board.square[FTK_C1].color == FTK_COLOR_WHITE) && 
                  (game->board.square[FTK_D1].type  == FTK_TYPE_ROOK) && 
                  (game->board.square[FTK_D1].color == FTK_COLOR_WHITE))
        {
          /* White is castled queenside */
          score += FBK_SCORE_CASTLED_QUEENSIDE;
        }
        else if ( (i == FTK_C8) && 
                  (game->board.square[FTK_C8].color == FTK_COLOR_BLACK) && 
                  (game->board.square[FTK_D8].type  == FTK_TYPE_ROOK) && 
                  (game->board.square[FTK_D8].color == FTK_COLOR_BLACK))
        {
          /* Black is castled queenside */
          score -= FBK_SCORE_CASTLED_QUEENSIDE;
        }
        break;
      }
      default:
      {
        break;
      }
    }
  }

  for(unsigned int i = 0; i < 8; i++)
  {
    if(white_pawns_on_file[i] > 1)
    {
      /* base*2^(doubled_pawn_count-2)) */
      score -= FBK_SCORE_DOUBLE_PAWN_BASE_PENALTY * (1 << (white_pawns_on_file[i]-2));
    }
    if(black_pawns_on_file[i] > 1)
    {
      /* base*2^(doubled_pawn_count-2)) */
      score += FBK_SCORE_DOUBLE_PAWN_BASE_PENALTY * (1 << (black_pawns_on_file[i]-2));
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

unsigned int position_repetition_count(const fbk_move_tree_node_s *node)
{
  FBK_ASSERT_MSG(node != NULL, "NULL node passed");
  FBK_ASSERT_MSG(node->hashed, "Node not hashed to check for threefold repetition");

  unsigned int repetition_count = 1;

  const fbk_move_tree_node_s *parent_node = node->parent;

  while(parent_node != NULL)
  {
    if(parent_node->hashed && (parent_node->key == node->key))
    {
      repetition_count++;
    }
    parent_node = parent_node->parent;
  }

  return repetition_count;
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
    fbk_hash_move_tree_node(node, game, true);

    ftk_update_board_masks(game);

    memset(&node->analysis_data, 0, sizeof(fbk_move_tree_node_analysis_data_s));

    /* Check for game end */
    node->analysis_data.result = ftk_check_for_game_end(game);
    node->analysis_data.best_child_result = node->analysis_data.result;

    if(FTK_END_NOT_OVER == node->analysis_data.result)
    {
      node->analysis_data.base_score = fbk_score_game(game);

      const unsigned int repetition_count = position_repetition_count(node);
      if(repetition_count == 2)
      {
        /* Approaching threefold repetition draw, divide score by 2 to approach 0 for a draw */
        node->analysis_data.base_score = (FBK_TWOFOLD_REPETITION_NUM * node->analysis_data.base_score) / FBK_TWOFOLD_REPETITION_DEN;
      }
      else if(repetition_count > 2)
      {
        /* Threefold repetition draw */
        node->analysis_data.base_score = 0;
        node->analysis_data.result = FTK_END_DRAW_REPETITION;
      }

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
      node->analysis_data.best_child_score = (FTK_COLOR_WHITE == game->turn)?FBK_SCORE_BLACK_MAX:FBK_SCORE_WHITE_MAX;

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
  FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Deleting move_tree_node node %p (%s->%s)", (void*) node, ftk_position_to_string_const_ptr(node->move.source), ftk_position_to_string_const_ptr(node->move.target));

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
  if(node_a->analysis_data.best_child_result == FTK_END_NOT_OVER)
  {
    fbk_score_t node_a_score = (node_a->analysis_data.best_child_index < node_a->child_count)?
                              (node_a->analysis_data.best_child_score):(node_a->analysis_data.base_score);

    if(node_b->analysis_data.best_child_result == FTK_END_NOT_OVER)
    {
      fbk_score_t node_b_score = (node_b->analysis_data.best_child_index < node_b->child_count)?
                                (node_b->analysis_data.best_child_score):(node_b->analysis_data.base_score);

      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.best_child_result))
    {
      /* If child depth is even, then node B will checkmate in the current turn's favor.  
         If odd, then node B will checkmate in the other turn's favor. */
      ret_val = ((node_b->analysis_data.best_child_depth % 2) == 0)?-1:1;
    }
    else if(FTK_END_DRAW(node_b->analysis_data.best_child_result))
    {
      fbk_score_t node_b_score = 0;
      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else
    {
      FBK_FATAL_MSG("Unhandled node game result");
    }
  }
  else if(FTK_END_DEFINITIVE(node_a->analysis_data.best_child_result))
  {
    if( (node_b->analysis_data.best_child_result == FTK_END_NOT_OVER) ||
        (FTK_END_DRAW(node_b->analysis_data.best_child_result)) )
    {
      /* If child depth is even, then node A will checkmate in the current turn's favor.  
         If odd, then node A will checkmate in the other turn's favor. */
      ret_val = ((node_a->analysis_data.best_child_depth % 2) == 0)?1:-1;
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.best_child_result))
    {
      if( ((node_a->analysis_data.best_child_depth % 2) == 0) &&
          ((node_b->analysis_data.best_child_depth % 2) == 1) )
      {
        /* Node A checkmates in the current turns favor.  Node B loses */
        ret_val = 1;
      }
      else if(((node_a->analysis_data.best_child_depth % 2) == 1) &&
              ((node_b->analysis_data.best_child_depth % 2) == 0) )
      {
        /* Node B checkmates in the current turns favor.  Node A loses */
        ret_val = -1;
      }
      else if(((node_a->analysis_data.best_child_depth % 2) == 1) &&
              ((node_b->analysis_data.best_child_depth % 2) == 1) )
      {
        /* Node A and Node B both lose. Choose the longest path to maximize the chance of an opponents mistake. */
        ret_val = node_a->analysis_data.best_child_depth-node_b->analysis_data.best_child_depth;
      }
      else if(((node_a->analysis_data.best_child_depth % 2) == 0) &&
              ((node_b->analysis_data.best_child_depth % 2) == 0) )
      {
        /* Node A and Node B both checkmate in the current turns favor. Choose the shortest path. */
          /* If node B is the larger path, return a positive value */
        ret_val = node_b->analysis_data.best_child_depth-node_a->analysis_data.best_child_depth;
      }
    }
    else
    {
      FBK_FATAL_MSG("Unhandled node game result");
    }
  }
  else if(FTK_END_DRAW(node_a->analysis_data.best_child_result))
  {
    fbk_score_t node_a_score = 0;

    if(node_b->analysis_data.best_child_result == FTK_END_NOT_OVER)
    {
      fbk_score_t node_b_score = (node_b->analysis_data.best_child_index < node_b->child_count)?
                                (node_b->analysis_data.best_child_score):(node_b->analysis_data.base_score);

      ret_val = (node_a->move.turn == FTK_COLOR_WHITE)?(node_a_score-node_b_score):(node_b_score-node_a_score);
    }
    else if(FTK_END_DEFINITIVE(node_b->analysis_data.best_child_result))
    {
      /* If child depth is even, then node B will checkmate in the current turn's favor.  
         If odd, then node B will checkmate in the other turn's favor. */
      ret_val = ((node_b->analysis_data.best_child_depth % 2) == 0)?-1:1;
    }
    else if(FTK_END_DRAW(node_b->analysis_data.best_child_result))
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
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Child node %u is not yet analyzed (%s->%s).  Aborting sort.", i, ftk_position_to_string_const_ptr(sorted_nodes[i]->move.source), ftk_position_to_string_const_ptr(sorted_nodes[i]->move.target));
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
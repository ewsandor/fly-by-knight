/*
 fly_by_knight_analysis.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Gama analysis for Fly by Knight
*/

#include <farewell_to_king.h>

#include "fly_by_knight_algorithm_constants.h"
#include "fly_by_knight_analysis.h"

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
  ftk_position_t   capture_square;
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
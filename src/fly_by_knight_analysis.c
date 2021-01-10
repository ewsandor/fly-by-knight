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
  unsigned int white_king_not_moved  = 0;
  unsigned int white_rooks_not_moved = 0;
  unsigned int black_king_not_moved  = 0;
  unsigned int black_rooks_not_moved = 0;

  for(i = 0; i < FTK_STD_BOARD_SIZE; i++)
  {
    advantage = (FTK_COLOR_WHITE == game->board.square[i].color)?1:-1;
    legal_move_count = ftk_get_num_bits_set(game->board.move_mask[i]);

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
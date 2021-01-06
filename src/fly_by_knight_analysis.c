/*
 fly_by_knight_analysis.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Gama analysis for Fly by Knight
*/

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
  fbk_score_t score;
  unsigned int i;
  int         advantage;

  for(i = 0; i < FTK_STD_BOARD_SIZE; i++)
  {
    advantage = (FTK_COLOR_WHITE == game->board.square[i].color)?1:-1;
    switch(game->board.square[i].type)
    {
      case FTK_TYPE_PAWN:
      {
        score += advantage*FBK_SCORE_PAWN;
        break;
      }
      case FTK_TYPE_KNIGHT:
      {
        score += advantage*FBK_SCORE_KNIGHT;
        break;
      }
      case FTK_TYPE_BISHOP:
      {
        score += advantage*FBK_SCORE_BISHOP;
        break;
      }
      case FTK_TYPE_ROOK:
      {
        score += advantage*FBK_SCORE_ROOK;
        break;
      }
      case FTK_TYPE_QUEEN:
      {
        score += advantage*FBK_SCORE_QUEEN;
        break;
      }
      case FTK_TYPE_KING:
      {
        score += advantage*FBK_SCORE_KING;
        break;
      }
      default:
      {
        break;
      }
    }
  }
  
  return score;
}
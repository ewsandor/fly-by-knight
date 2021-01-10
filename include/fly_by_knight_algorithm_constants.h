/*
 fly_by_knight_algorithm_constants.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Algorithm constants for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_ALGORITHM_CONSTANTS_H_
#define _FLY_BY_KNIGHT_ALGORITHM_CONSTANTS_H_

/* Weighting for piece types */
#define FBK_SCORE_PAWN      1000
#define FBK_SCORE_KNIGHT    3000
#define FBK_SCORE_BISHOP    3000
#define FBK_SCORE_ROOK      5000
#define FBK_SCORE_QUEEN     9000
#define FBK_SCORE_KING   1000000

#define FBK_SCORE_WHITE_MAX  (64  * FBK_SCORE_KING)
#define FBK_SCORE_BLACK_MAX  (-64 * FBK_SCORE_KING)

#endif //_FLY_BY_KNIGHT_ALGORITHM_CONSTANTS_H_
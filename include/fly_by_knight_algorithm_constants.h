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

/* Max score for each color */
#define FBK_SCORE_WHITE_MAX  (64  * FBK_SCORE_KING)
#define FBK_SCORE_BLACK_MAX  (-64 * FBK_SCORE_KING)

/* Weight per legal move for piece */
#define FBK_SCORE_PAWN_MOVE    1
#define FBK_SCORE_KNIGHT_MOVE  3
#define FBK_SCORE_BISHOP_MOVE  3
#define FBK_SCORE_ROOK_MOVE    5
#define FBK_SCORE_QUEEN_MOVE   9
#define FBK_SCORE_KING_MOVE   10

/* Score for the ability to castle */
#define FBK_SCORE_CAN_CASTLE (FBK_SCORE_PAWN/3)

#endif //_FLY_BY_KNIGHT_ALGORITHM_CONSTANTS_H_
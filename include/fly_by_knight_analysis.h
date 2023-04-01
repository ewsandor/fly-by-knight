/*
 fly_by_knight_analysis.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Core game analysis for Fly by Knight
*/

#ifndef __FLY_BY_KNIGHT_ANALYSIS_H__
#define __FLY_BY_KNIGHT_ANALYSIS_H__

#include "fly_by_knight_types.h"

/**
 * @brief Score game position for white or black advantage
 * 
 * @param game      to analyze
 * @return fbk_score_t 
 */
fbk_score_t fbk_score_game(const ftk_game_s * game);

/**
 * @brief Evaluates node represented by given game
 * 
 * @param node Node to evaluate
 * @param game Game representing this node (Assumes move is already applied)
 */
void fbk_evaluate_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game);

/**
 * @brief Clears evaluation and deletes all child nodes
 * 
 * @param node 
 */
void fbk_unevaluate_move_tree_node(fbk_move_tree_node_s * node);

/**
 * @brief Evaluates all children of node
 * 
 * @param node 
 * @param game 
 */
void fbk_evaluate_move_tree_node_children(fbk_move_tree_node_s * node, ftk_game_s game);

#endif //__FLY_BY_KNIGHT_ANALYSIS_H__
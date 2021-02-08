/*
 fly_by_knight_analysis.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Gama analysis for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_ANALYSIS_H_
#define _FLY_BY_KNIGHT_ANALYSIS_H_

#include "fly_by_knight_types.h"

/**
 * @brief Initialized Fly by Knight analysis data
 * 
 * @param fbk 
 * @return true if successful
 */
bool fbk_init_analysis_data(fbk_instance_s *fbk);

/**
 * @brief Score game position for white or black advantage
 * 
 * @param game      to analyze
 * @return fbk_score_t 
 */
fbk_score_t fbk_score_game(const ftk_game_s * game);

/**
 * @brief Evaluates all children of node
 * 
 * @param node 
 * @param game 
 */
void fbk_evaluate_move_tree_node_children(fbk_move_tree_node_s * node, ftk_game_s game);

#endif //_FLY_BY_KNIGHT_ANALYSIS_H_
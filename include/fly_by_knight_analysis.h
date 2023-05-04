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
 * @brief Initialized Look-Up Tables for Fly by Knight analysis
 * 
 * @return true if successful
 */
bool fbk_init_analysis_lut();


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
 * @param locked True if caller is holding the node's lock, else lock will be obtained
 * 
 * @return true if node was evaluated now, false if node is invalid or previously evaluated
 */
bool fbk_evaluate_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game, bool locked);

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

/**
 * @brief Compares the score of two evaluated move tree nodes
 * 
 * @param a Node A for comparison
 * @param b Node B for comparison
 * 
 * @return >0 if A is better than B, <0 if B is better than A, or 0 if equivalent
*/
int fbk_compare_move_tree_nodes(const void *a, const void *b);

/**
 * @brief Sort child nodes of given node.  Assumes caller holds the lock on node
 * 
 * @param node         node to sort children
 * @param sorted_nodes output array of sorted node pointers.  Must be size of node->child_count node pointers
 */
bool fbk_sort_child_nodes(const fbk_move_tree_node_s * node, fbk_move_tree_node_s* sorted_nodes[]);

#endif //__FLY_BY_KNIGHT_ANALYSIS_H__
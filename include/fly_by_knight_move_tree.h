/*
 fly_by_knight_move_tree.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_MOVE_TREE_H_
#define _FLY_BY_KNIGHT_MOVE_TREE_H_

#include "fly_by_knight_types.h"

/**
 * @brief Initializes node with given move.  If NULL move passed, 
 * 
 * @param node   Node to be initialized
 * @param parent Parent node, NULL if root
 * @param move   Move to init node with, NULL if root
 */
void fbk_init_move_tree_node(fbk_move_tree_node_s * node, fbk_move_tree_node_s * parent, const ftk_move_s * move);

/**
 * @brief Applies move to given game
 * 
 * @param node Node to apply
 * @param game Game to modify with move tree node
 * @return     True if successful
 */
bool fbk_apply_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game);

/**
 * @brief Reverts move from given game
 * 
 * @param node Node to undo
 * @param game Game to modify with move tree node
 * @return     True if successful
 */
bool fbk_undo_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game);

/**
 * @brief Evaluates node represented by given game
 * 
 * @param node Node to evaluate
 * @param game Game representing this node (Assumes move is already applied)
 */
void fbk_evaluate_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game);

#endif //_FLY_BY_KNIGHT_MOVE_TREE_H_
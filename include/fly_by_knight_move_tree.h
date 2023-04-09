/*
 fly_by_knight_move_tree.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#ifndef __FLY_BY_KNIGHT_MOVE_TREE_H__
#define __FLY_BY_KNIGHT_MOVE_TREE_H__

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
 * @brief Releases memory for node and all child nodes
 * @param fast true to perform fast delete (skip invalidating) if not re-initializing node
 * 
 * @param node node to delete
 */
void fbk_delete_move_tree_node(fbk_move_tree_node_s * node, bool fast);

/**
 * @brief Releases memory for node and all child nodes using multiple threads
 * 
 * @param node node to delete
 * @param fast true to perform fast delete (skip invalidating) if not re-initializing node
 */
void fbk_delete_move_tree_node_parallel(fbk_move_tree_node_s * node);

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
 * @brief Returns child node for given move, NULL if no child node for move or current node is not evaluated
 * 
 * @param current_node 
 * @param move 
 * @return fbk_move_tree_node_s* 
 */
fbk_move_tree_node_s * fbk_get_move_tree_node_for_move(fbk_move_tree_node_s * current_node, ftk_move_s * move);

/**
 * @brief Compresses given move tree node
 * @param node   Node to compress
 * @param locked True if caller is holding the node's lock, else lock will be obtained
 * 
 * @return true if decompressed now, false if already decompressed before calling.
*/
bool fbk_compress_move_tree_node(fbk_move_tree_node_s * node, bool locked);

/**
 * @brief Decompresses given move tree node
 * @param node   Node to compress
 * @param locked True if caller is holding the node's lock, else lock will be obtained
 * 
 * @return true if decompressed now, false if already decompressed before calling.
*/
bool fbk_decompress_move_tree_node(fbk_move_tree_node_s * node, bool locked);

#endif //_FLY_BY_KNIGHT_MOVE_TREE_H_
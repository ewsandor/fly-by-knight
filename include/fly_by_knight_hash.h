/*
 fly_by_knight_hash.h
 Fly by Knight - Chess Engine
 Edward Sandor
 April 2023
 
 Hashing logic for Fly by Knight
*/

#include "fly_by_knight_types.h"

#ifndef __FLY_BY_KNIGHT_HASH_H__
#define __FLY_BY_KNIGHT_HASH_H__

/**
 * @brief Hash node represented by given game
 * 
 * @param node Node to hash
 * @param game Game representing this node (Assumes move is already applied)
 * @param locked True if caller is holding the node's lock, else lock will be obtained
 * 
 * @return true if node was hashed now, false if node is invalid or previously hashed 
 */
bool fbk_hash_move_tree_node(fbk_move_tree_node_s * node, const ftk_game_s * game, bool locked);

#endif /* __FLY_BY_KNIGHT_HASH_H__ */
/*
 fly_by_knight_move_tree.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#include <farewell_to_king.h>

#include "fly_by_knight.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_move_tree.h"


/**
 * @brief Initializes node with given move.  If NULL move passed, 
 * 
 * @param node   Node to be initialized
 * @param parent Parent node, NULL if root
 * @param move   Move to init node with, NULL if root
 */
void fbk_init_move_tree_node(fbk_move_tree_node_s * node, fbk_move_tree_node_s * parent, ftk_move_s * move)
{
  FBK_ASSERT_MSG(node != NULL, "NULL node passed");

  FBK_ASSERT_MSG(true == fbk_mutex_init(&node->lock), "Failed to init node mutex");
  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");

  node->parent = parent;

  if(move && FTK_MOVE_VALID(*move))
  {
    node->move = *move;
  }
  else
  {
    ftk_invalidate_move(&node->move);
  }
  
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to lock node mutex");
}
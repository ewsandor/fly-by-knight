/*
 fly_by_knight_move_tree.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>

#include "fly_by_knight.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_move_tree.h"

/**
 * @brief Initializes node with given move.  If NULL move passed, 
 * 
 * @param node   Node to be initialized
 * @param parent Parent node, NULL if root
 * @param move   Move to init node with, NULL if root
 */
void fbk_init_move_tree_node(fbk_move_tree_node_s * node, fbk_move_tree_node_s * parent, const ftk_move_s * move)
{
  FBK_ASSERT_MSG(node != NULL, "NULL node passed");

  memset(node, 0, sizeof(fbk_move_tree_node_s));

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
  
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
}

/**
 * @brief Releases memory for node and all child nodes
 * 
 * @param node 
 */
void fbk_delete_move_tree_node(fbk_move_tree_node_s * node)
{
  fbk_unevaluate_move_tree_node(node);

  FBK_ASSERT_MSG(true == fbk_mutex_destroy(&node->lock), "Failed to destroy node mutex");

  memset(node, 0, sizeof(fbk_move_tree_node_s));
}

/**
 * @brief Applies move to given game
 * 
 * @param node Node to apply
 * @param game Game to modify with move tree node
 */
bool fbk_apply_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game)
{
  ftk_result_e result;
  ftk_move_s   move;

  FBK_ASSERT_MSG(game != NULL, "Null game passed");
  FBK_ASSERT_MSG(node != NULL, "Null node passed");

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  move = node->move;
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");

  result = ftk_move_forward_quick(game, &move);

  return (FTK_SUCCESS == result);
}

/**
 * @brief Reverts move from given game
 * 
 * @param node Node to undo
 * @param game Game to modify with move tree node
 */
bool fbk_undo_move_tree_node(fbk_move_tree_node_s * node, ftk_game_s * game)
{
  ftk_result_e result;
  ftk_move_s   move;

  FBK_ASSERT_MSG(game != NULL, "Null game passed");
  FBK_ASSERT_MSG(node != NULL, "Null node passed");

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  move = node->move;
  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");

  result = ftk_move_backward_quick(game, &move);

  return (FTK_SUCCESS == result);
}

/**
 * @brief Returns child node for given move, NULL if no child node for move or current node is not evaluated
 * 
 * @param current_node 
 * @param move 
 * @return fbk_move_tree_node_s* 
 */
fbk_move_tree_node_s * fbk_get_move_tree_node_for_move(fbk_move_tree_node_s * current_node, ftk_move_s * move)
{
  unsigned int i;
  fbk_move_tree_node_s * ret_node = NULL;

  if(move && FTK_MOVE_VALID(*move))
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&current_node->lock), "Failed to lock node mutex");
    if(current_node->evaluated)
    {
      for(i = 0; ((i < current_node->child_count) && (ret_node == NULL)); i++)
      {
        FBK_ASSERT_MSG(true == fbk_mutex_lock(&current_node->child[i].lock), "Failed to lock node mutex");
        if(FTK_COMPARE_MOVES(current_node->child[i].move, *move))
        {
          ret_node = &current_node->child[i];
        }
        FBK_ASSERT_MSG(true == fbk_mutex_unlock(&current_node->child[i].lock), "Failed to unlock node mutex");
      }
    }
    FBK_ASSERT_MSG(true == fbk_mutex_unlock(&current_node->lock), "Failed to unlock node mutex");
  }

  return ret_node;
}
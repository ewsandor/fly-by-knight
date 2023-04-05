/*
 fly_by_knight_move_tree.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>
#include <zlib.h>

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
    fbk_decompress_move_tree_node(current_node, true);
    if(current_node->analysis_data.evaluated)
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

#define ZLIB_CHUNK_SIZE        (FBK_MOVE_TREE_MAX_NODE_COUNT*sizeof(fbk_move_tree_node_s))
#define ZLIB_COMPRESSION_LEVEL Z_DEFAULT_COMPRESSION
/* deflate memory usage (bytes) = (1 << (windowBits+2)) + (1 << (memLevel+9)) + 6 KB */
/* From zlib manual:
    The windowBits parameter is the base two logarithm of the window size (the size of the history buffer). It should be in the range 8..15 for this version of the library. Larger values of this parameter result in better compression at the expense of memory usage. The default value is 15 if deflateInit is used instead.
    For the current implementation of deflate(), a windowBits value of 8 (a window size of 256 bytes) is not supported. As a result, a request for 8 will result in 9 (a 512-byte window). In that case, providing 8 to inflateInit2() will result in an error when the zlib header with 9 is checked against the initialization of inflate(). The remedy is to not use 8 with deflateInit2() with this initialization, or at least in that case use 9 with inflateInit2(). */
#define ZLIB_WINDOW_BITS       15
#define ZLIB_MEM_LEVEL         (ZLIB_WINDOW_BITS-7)
bool fbk_compress_move_tree_node(fbk_move_tree_node_s * node, bool locked)
{
  bool ret_val = true;

  FBK_ASSERT_MSG(node != NULL, "Null node passed");
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  }

  if((node->child_count > 0) && (node->child != NULL))
  {
    for(fbk_move_tree_node_count_t i = 0; i < node->child_count; i++)
    {
      /* Invalidate child parent node as it may not be valid after decompressed (e.g. parent was also compressed) */
      FBK_ASSERT_MSG(node->child[i].parent == node, "Child node (%p) parent is not this node (%p).", (void*)node->child[i].parent, (void*)node);
      node->child[i].parent = NULL;
    }

    FBK_ASSERT_MSG(node->child_compressed == NULL, "Both child and child compressed set.");

    /* allocate deflate state */
    Bytef out[ZLIB_CHUNK_SIZE];
    z_stream strm;
    strm.zalloc = Z_NULL;
    strm.zfree  = Z_NULL;
    strm.opaque = Z_NULL;
    int ret = deflateInit2(&strm, ZLIB_COMPRESSION_LEVEL, Z_DEFLATED, ZLIB_WINDOW_BITS, ZLIB_MEM_LEVEL, Z_DEFAULT_STRATEGY);

    FBK_ASSERT_MSG(Z_OK == ret, "Error (%d) initializing deflate stream.\n", ret);

    strm.avail_in = node->child_count*sizeof(fbk_move_tree_node_s);
    strm.next_in = (Bytef*) node->child;

    FBK_ASSERT_MSG(strm.avail_in < ZLIB_CHUNK_SIZE, "Child buffer larger than expected (%u bytes)", strm.avail_in);

    unsigned int output_bytes = 0, next_output_index = 0;
    do
    {
      strm.avail_out = ZLIB_CHUNK_SIZE;
      strm.next_out  = out;

      ret = deflate(&strm, Z_FINISH);    /* no bad return value */
      FBK_ASSERT_MSG(ret != Z_STREAM_ERROR, "zlib deflate error.");  /* state not clobbered */

      unsigned have = ZLIB_CHUNK_SIZE - strm.avail_out;
      output_bytes += have;
      node->child_compressed = (fbk_move_tree_node_s*) realloc(node->child_compressed, output_bytes);
      memcpy(&((Bytef*)node->child_compressed)[next_output_index], out, have);
      next_output_index += output_bytes;

    } while(strm.avail_out == 0);

    FBK_ASSERT_MSG(strm.avail_in == 0, "Incomplete deflate.");
    FBK_ASSERT_MSG(ret == Z_STREAM_END, "Deflate in bad state %u", ret);

    node->child_compressed_size = output_bytes;
    free(node->child);
    node->child = NULL;
    deflateEnd(&strm);
  }
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
  }

  return ret_val;
}

bool fbk_decompress_move_tree_node(fbk_move_tree_node_s * node, bool locked)
{
  bool ret_val = true;

  FBK_ASSERT_MSG(node != NULL, "Null node passed");
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  }

  if((node->child_compressed_size > 0) && (node->child_compressed != NULL))
  {
    FBK_ASSERT_MSG(node->child == NULL, "Both child and child compressed set.");

    unsigned int have;
    z_stream strm;
    unsigned char out[ZLIB_CHUNK_SIZE];

    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    strm.avail_in = 0;
    strm.next_in = Z_NULL;
    int ret = inflateInit(&strm);
    FBK_ASSERT_MSG(Z_OK == ret, "Error (%d) initializing inflate stream.\n", ret);

    strm.avail_in = node->child_compressed_size;
    strm.next_in  = node->child_compressed;

    unsigned int output_bytes = 0, next_output_index = 0;
    do
    {
      strm.avail_out = ZLIB_CHUNK_SIZE;
      strm.next_out = out;

      ret = inflate(&strm, Z_NO_FLUSH);
      switch (ret) {
        case Z_OK:
        case Z_STREAM_END:
        {
          break;
        }
        default:
        {
          FBK_FATAL_MSG("Node inflation error %u", ret);
          break;
        }
      }
      have = ZLIB_CHUNK_SIZE - strm.avail_out;
      output_bytes += have;
      node->child = (fbk_move_tree_node_s*) realloc(node->child, output_bytes);
      memcpy(&((Bytef*)node->child)[next_output_index], out, have);
      next_output_index += output_bytes;

    } while (strm.avail_out == 0);

    FBK_ASSERT_MSG(strm.avail_in == 0, "Incomplete inflate.");
    FBK_ASSERT_MSG(ret == Z_STREAM_END, "Inflate in bad state %u", ret);
    FBK_ASSERT_MSG(output_bytes == node->child_count*sizeof(fbk_move_tree_node_s), "Unexpected inflation size %u.", output_bytes);
    
    free(node->child_compressed);
    node->child_compressed      = NULL;
    node->child_compressed_size = 0;
    inflateEnd(&strm);

    for(fbk_move_tree_node_count_t i = 0; i < node->child_count; i++)
    {
      /* Update child parent node as it may not be valid after decompressed (e.g. parent was also compressed) */
      FBK_ASSERT_MSG(node->child[i].parent == NULL, "Parent node was not cleared; this node is at %p, but parent points at %p.", (void*) node, (void*) node->child[i].parent);
      node->child[i].parent = node;
    }
  }
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to unlock node mutex");
  }

  return ret_val;
}
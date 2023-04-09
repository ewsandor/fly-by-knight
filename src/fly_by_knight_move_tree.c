/*
 fly_by_knight_move_tree.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 Move Tree manipulation for Fly by Knight
*/

#include <string.h>
#include <errno.h>

#include <farewell_to_king.h>
#ifdef FBK_ZLIB_COMPRESSION
#include <zlib.h>
#endif

#include "fly_by_knight.h"
#include "fly_by_knight_analysis.h"
#include "fly_by_knight_debug.h"
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

void fbk_delete_move_tree_node(fbk_move_tree_node_s * node, bool fast)
{
  fbk_unevaluate_move_tree_node(node, fast);

  FBK_ASSERT_MSG(true == fbk_mutex_destroy(&node->lock), "Failed to destroy node mutex");

  if(!fast)
  {
    memset(node, 0, sizeof(fbk_move_tree_node_s));
  }
}

typedef struct
{
  unsigned int          index;
  bool                  parallel;
  fbk_move_tree_node_s *node;
  bool                  fast;

} delete_thread_arg_s;

void * delete_thread_f(void* arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL arg passed");
  delete_thread_arg_s *delete_thread_arg = (delete_thread_arg_s *) arg;

  if(delete_thread_arg->parallel)
  {
    fbk_delete_move_tree_node_parallel(delete_thread_arg->node);
  }
  else
  {
    fbk_delete_move_tree_node(delete_thread_arg->node, delete_thread_arg->fast);
  }

  return NULL;
}

static int node_depth_cmp(const void *a, const void *b)
{
  int ret_val = 0;

  const fbk_move_tree_node_s *node_a = *((fbk_move_tree_node_s **) a);
  const fbk_move_tree_node_s *node_b = *((fbk_move_tree_node_s **) b);

  fbk_depth_t node_a_depth = (node_a->analysis_data.evaluated)?(node_a->analysis_data.max_depth):0;
  fbk_depth_t node_b_depth = (node_b->analysis_data.evaluated)?(node_b->analysis_data.max_depth):0;

  ret_val = (node_a_depth-node_b_depth);

  return ret_val;
}


/**
 * @brief Releases memory for node and all child nodes using multiple threads
 * 
 * @param node node to delete
 */
void fbk_delete_move_tree_node_parallel(fbk_move_tree_node_s * node)
{
  FBK_ASSERT_MSG(node != NULL, "NULL node passed");

  FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  fbk_decompress_move_tree_node(node, true);

  if(node->child_count > 0)
  {
    fbk_move_tree_node_s** sorted_nodes = malloc(node->child_count * sizeof(fbk_move_tree_node_s*));
    FBK_ASSERT_MSG(sorted_nodes != NULL, "Failed to allocate memory for sorted_nodes buffer");

    /* Prep child nodes and initialize pointers */
    for(fbk_move_tree_node_count_t i = 0; i < node->child_count; i++)
    {
      sorted_nodes[i] = &node->child[i];
      fbk_mutex_lock(&sorted_nodes[i]->lock);
    }
    qsort(sorted_nodes, node->child_count, sizeof(fbk_move_tree_node_s*), node_depth_cmp);

    pthread_t           *delete_thread     = calloc(node->child_count, sizeof(pthread_t));
    delete_thread_arg_s *delete_thread_arg = calloc(node->child_count, sizeof(delete_thread_arg_s));

    for(int i = 0; i < node->child_count; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Starting thread to delete child node %u.", i);
      delete_thread_arg[i].index    = i;
      delete_thread_arg[i].parallel = (node->child_count < 4) || (i >= (node->child_count-4));
      delete_thread_arg[i].node     = sorted_nodes[i];
      delete_thread_arg[i].fast     = true;
      fbk_mutex_unlock(&sorted_nodes[i]->lock);

      int rc = 0;
      do
      {
        rc = pthread_create(&delete_thread[i], NULL, delete_thread_f, &delete_thread_arg[i]);
      } while(rc == EAGAIN);
      FBK_ASSERT_MSG(rc == 0, "Error (%d) creating delete thread %u", rc, i);
    }

    for(fbk_move_tree_node_count_t i = 0; i < node->child_count; i++)
    {
      FBK_ASSERT_MSG(0 == pthread_join(delete_thread[i], NULL), "Error joining delete thread %u", i);
      FBK_DEBUG_MSG(FBK_DEBUG_MIN, "Thread to delete child node %u done.", i);
    }

    free(delete_thread);
    free(delete_thread_arg);
    free(sorted_nodes);

    free(node->child);
    node->child_count = 0;
    node->child       = NULL;
  }

  FBK_ASSERT_MSG(true == fbk_mutex_unlock(&node->lock), "Failed to lock node mutex");
  fbk_mutex_destroy(&node->lock);
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
  bool ret_val = false;

#ifdef FBK_ZLIB_COMPRESSION
  FBK_ASSERT_MSG(node != NULL, "Null node passed");
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  }

  if((node->child_count > 0) && (node->child != NULL))
  {
    ret_val = true;
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
#else
  FBK_UNUSED(node);
  FBK_UNUSED(locked);
#endif /* FBK_ZLIB_COMPRESSION */

  return ret_val;
}

bool fbk_decompress_move_tree_node(fbk_move_tree_node_s * node, bool locked)
{
  bool ret_val = false;

#ifdef FBK_ZLIB_COMPRESSION
  FBK_ASSERT_MSG(node != NULL, "Null node passed");
  if(!locked)
  {
    FBK_ASSERT_MSG(true == fbk_mutex_lock(&node->lock), "Failed to lock node mutex");
  }

  if((node->child_compressed_size > 0) && (node->child_compressed != NULL))
  {
    ret_val = true;
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
#else
  FBK_UNUSED(node);
  FBK_UNUSED(locked);
#endif /* FBK_ZLIB_COMPRESSION */

  return ret_val;
}
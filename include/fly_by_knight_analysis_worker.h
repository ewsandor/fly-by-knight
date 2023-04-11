/*
 fly_by_knight_analysis_worker.h
 Fly by Knight - Chess Engine
 Edward Sandor
 March 2023
 
 Gama analysis worker logic for Fly by Knight
*/

#ifndef __FLY_BY_KNIGHT_ANALYSIS_WORKER_H__
#define __FLY_BY_KNIGHT_ANALYSIS_WORKER_H__

#include "fly_by_knight_types.h"

typedef uint_fast32_t fbk_analysis_node_count_t;

typedef enum
{
  FBK_ANALYSIS_JOB_COMPLETE,
  FBK_ANALYSIS_JOB_ABORTED,
  FBK_ANALYSIS_JOB_NO_LOCK,

} fbk_analysis_job_result_e;

/* Data to be returned by job processing logic */
typedef struct 
{
  fbk_analysis_job_result_e result;

} fbk_analysis_job_result_s;

/* Data to be passed between recursive calls of job processing logic */
typedef struct 
{
  /* Thread index processing this job for logging */
  fbk_thread_index_t        thread_index;

  /* Indicates if this is the top call or a recursive call */
  bool                      top_call;

  /* Number of nodes evaluated by this job */
  fbk_analysis_node_count_t nodes_evaluated;

} fbk_analysis_job_context_s;

typedef unsigned int fbk_analysis_job_id_t;

/* Job details for worker thread to analyze game */
typedef struct 
{
  /* Identifier for job */
  fbk_analysis_job_id_t      job_id;
  /* Reference game to begin analysis on */
  ftk_game_s                 game;
  /* Node to begin analysis on */
  fbk_move_tree_node_s      *node;
  /* Depth to search */
  fbk_depth_t                depth;
  /* Breadth to search */
  fbk_breadth_t              breadth;
} fbk_analysis_job_s;

/* Node for job queue */
typedef struct fbk_analysis_queue_node_struct fbk_analysis_job_queue_node_s;

struct fbk_analysis_queue_node_struct
{
  /* Job stored by node */
  fbk_analysis_job_s job;

  /* Job stored by node */
  fbk_analysis_job_queue_node_s *next_job;
};

/* Type for tracking job count */
typedef uint_fast16_t fbk_analysis_job_count_t;

/* Analysis job queue structure (linked list) */
typedef struct 
{
  /* Lock for accessing the queue */
  fbk_mutex_t lock;
  /* Condition when new job is available*/
  pthread_cond_t new_job_available;
  /* Condition when new job is available*/
  pthread_cond_t job_claimed;
  /* Condition when new job processing is stopped (either successfully or in failure) */
  pthread_cond_t job_ended;

  /* Indicate the job queue has been cleared and need new initial jobs */
  bool                           queue_cleared;
  /* Number of active jobs popped from the queue but not yet freed */
  fbk_analysis_job_count_t       active_job_count;

  /* Number of queued jobs */
  fbk_analysis_job_count_t       job_count;
  /* Root job of queue */
  fbk_analysis_job_queue_node_s *next_job;
  /* Back of job queue*/
  fbk_analysis_job_queue_node_s *last_job;

  /* ID for next job created*/
  fbk_analysis_job_id_t          next_job_id;

} fbk_analysis_job_queue_s;

/* Type for counting worker threads */
typedef uint_fast16_t fbk_worker_thread_count_t;

typedef struct 
{
  /* Analysis check protection*/
  fbk_mutex_t           lock;
  pthread_cond_t        analysis_started_cond;
  /* Indicates analysis has been requested and is active */
  bool                  analysis_active;

  /* Root node for analysis */
  fbk_move_tree_node_s *root_node;
  /* Reference game for analysis */
  ftk_game_s            game;

} fbk_analysis_state_s;

/* Data for worker threads */
typedef struct 
{
  /*Index for this worker thread */
  fbk_thread_index_t        thread_index;

  /* Pointer to job queue associated with this worker thread */
  fbk_analysis_job_queue_s *job_queue;

  /* Analysis state information */
  fbk_analysis_state_s * analysis_state;

  /* Thread Handle */
  pthread_t worker_thread;

} fbk_worker_thread_data_s;

/* Structure for storing analysis statistics */
typedef struct 
{
  /* Lock for accessing analysis statistics */
  fbk_mutex_t lock;

  /* Nodes analyzed since analysis start */
  fbk_analysis_node_count_t analyzed_nodes;
  /* Nodes analyzed since game start */
  fbk_analysis_node_count_t game_analyzed_nodes;
  /* Nodes analyzed since process start */
  fbk_analysis_node_count_t total_analyzed_nodes;

} fbk_analysis_stats_s;

/* Root structure for Fly by Knight analysis data */
typedef struct 
{
  /* Indicates if analysis data has been initialized */
  bool                      initialized;

  fbk_instance_s           *fbk;

  /* Analysis state information */
  fbk_analysis_state_s      analysis_state;

  /* Analysis statistics */
  fbk_analysis_stats_s      analysis_stats;

  /* Manager thread to manage analysis worker threads */
  pthread_t                 worker_manager_thread;

  /* Worker thread count */
  fbk_worker_thread_count_t worker_thread_count;

  /* Array of worker threads */
  fbk_worker_thread_data_s  **worker_thread_data;

  /* Queue of analysis jobs for worker threads*/
  fbk_analysis_job_queue_s  job_queue;

} fbk_analysis_data_s;



/**
 * @brief Initialized Fly by Knight analysis data
 * 
 * @param fbk 
 * @return true if successful
 */
bool fbk_init_analysis_data(fbk_instance_s *fbk);

/**
 * @brief Updates the configured number of worker threads
 * 
 * @param count new number of worker threads to allow
*/
void fbk_update_worker_thread_count(unsigned int count);

/**
 * @brief Starts analysis at given move tree node
 * @param node Move tree node of interest
*/
void fbk_start_analysis(const ftk_game_s *game, fbk_move_tree_node_s * node);

/**
 * @brief Stops analysis and blocks until all analysis has stopped
 * @param clear_pending_jobs option to clear the job queue after stopping
 * 
 * @return true if analysis was started before calling, false if analysis was already stopped
*/
bool fbk_stop_analysis(bool clear_pending_jobs);

#endif /* __FLY_BY_KNIGHT_ANALYSIS_WORKER_H__ */
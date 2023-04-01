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

/* Job details for worker thread to analyze game */
typedef struct 
{
  /* Node to begin analysis on */
  fbk_move_tree_node_s * node;
  /* Maximum depth to search */
  fbk_depth_t            depth;
  /* Maximum breadth to search */
  fbk_breadth_t          breadth;
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

  /* Number of queued jobs */
  fbk_analysis_job_count_t job_count;
  /* Root job of queue */
  fbk_analysis_job_queue_node_s *next_job;
  /* Back of job queue*/
  fbk_analysis_job_queue_node_s *last_job;

} fbk_analysis_job_queue_s;

/* Type for counting worker threads */
typedef uint_fast16_t fbk_worker_thread_count_t;

typedef struct 
{
  /* Analysis check protection*/
  fbk_mutex_t    lock;
  pthread_cond_t analysis_started_cond;
  /* Indicates analysis has been requested and is active */
  bool           analysis_active;

} fbk_analysis_state_s;

/* Data for worker threads */
typedef struct 
{
  /*Identifier for this worker thread */
  unsigned int thread_id;

  /* Pointer to job queue associated with this worker thread */
  fbk_analysis_job_queue_s *job_queue;

  /* Analysis state information */
  fbk_analysis_state_s * analysis_state;

  /* Thread Handle */
  pthread_t worker_thread;

} fbk_worker_thread_data_s;

typedef uint_fast32_t fbk_analysis_node_count_t;

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

  /* Worker thread count */
  fbk_worker_thread_count_t worker_thread_count;

  /* Array of worker threads */
  fbk_worker_thread_data_s  *worker_thread_data;

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

#endif /* __FLY_BY_KNIGHT_ANALYSIS_WORKER_H__ */
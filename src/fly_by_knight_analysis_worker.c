/*
 fly_by_knight_analysis_worker.c
 Fly by Knight - Chess Engine
 Edward Sandor
 March 2023
 
 Gama analysis worker logic for Fly by Knight
*/

#include <string.h>
#include <unistd.h>

#include "fly_by_knight_analysis_worker.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"

fbk_analysis_data_s fbk_analysis_data = {0};

static void worker_thread_analysis_state_cleanup(void *arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_analysis_state_s *analysis_state = (fbk_analysis_state_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread 0x%lx releasing analysis state mutex for thread cancellation.", pthread_self());
  fbk_mutex_unlock(&analysis_state->lock);
}

static void wait_for_analysis_start(fbk_analysis_state_s *analysis_state)
{
    /* Check if analysis is active */
    fbk_mutex_lock(&analysis_state->lock);
    pthread_cleanup_push(worker_thread_analysis_state_cleanup, &analysis_state->lock);
    while(!analysis_state->analysis_active)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Thread 0x%lx waiting for analysis to start.", pthread_self());
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&analysis_state->analysis_started_cond, &analysis_state->lock),
        "Failed Waiting for analysis started condition.");
    }
    pthread_cleanup_pop(0);
    fbk_mutex_unlock(&analysis_state->lock);
}

/**
 * @brief Logic to add job to back of job queue.  Assumed caller has lock.
 * @param queue   queue to append
 * @param new_job new job to append
*/
void push_job_to_job_queue(fbk_analysis_job_queue_s * queue, fbk_analysis_job_queue_node_s * new_job)
{

  FBK_ASSERT_MSG(queue != NULL,   "NULL job queue passed.");
  FBK_ASSERT_MSG(new_job != NULL, "NULL job passed.");

  /* Ensure end of queue is NULL */
  new_job->next_job = NULL;

  if(queue->job_count > 0)
  {
    /* Add to linked list */
    queue->last_job->next_job = new_job;
    queue->last_job = new_job;
  }
  else
  {
    /* Init root job */
    queue->next_job = new_job;
    queue->last_job = new_job;
  }

  queue->job_count++;
  FBK_ASSERT_MSG(0 == pthread_cond_signal(&queue->new_job_available), "Failed to signal new job is available.");
}

#define WORKER_MANAGER_QUEUED_JOBS_PER_WORKER 2
/**
 * @brief Main thread for worker manager thread
 * @param arg pointer to main analysis data structure
*/
static void * worker_manager_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_analysis_data_s *analysis_data = (fbk_analysis_data_s *) arg;

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Starting worker thread manager with ID 0x%lx.", pthread_self());

  while(1)
  {
    wait_for_analysis_start(&analysis_data->analysis_state);

    fbk_mutex_lock(&analysis_data->job_queue.lock);
    while(analysis_data->job_queue.job_count >= WORKER_MANAGER_QUEUED_JOBS_PER_WORKER*analysis_data->fbk->config.worker_threads)
    {
      /* Wait for some jobs to be claimed before queueing new jobs */
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&analysis_data->job_queue.job_claimed, &analysis_data->job_queue.lock),
        "Failed to check condition that a job has been claimed");
    }

    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Pushing job to job queue.");
    fbk_analysis_job_queue_node_s *new_job = malloc(sizeof(fbk_analysis_job_queue_node_s));
    FBK_ASSERT_MSG(new_job != NULL, "Failed to allocate memory for new job.");
    /* Fill job info here... */
    push_job_to_job_queue(&analysis_data->job_queue, new_job);

    fbk_mutex_unlock(&analysis_data->job_queue.lock);
  }

  FBK_NO_RETURN
  return NULL;
}

/**
 * @brief Initialized Fly by Knight analysis data
 * 
 * @param fbk 
 * @return true if successful
 */
bool fbk_init_analysis_data(fbk_instance_s *fbk)
{
  bool ret_val = true;

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Initializing analysis data");

  FBK_ASSERT_MSG(fbk != NULL, "NULL FBK instance passed");

  if(false == fbk_analysis_data.initialized)
  {
    memset(&fbk_analysis_data, 0, sizeof(fbk_analysis_data_s));

    fbk_analysis_data.fbk = fbk;

    /* Initialize analysis state */
    fbk_analysis_data.analysis_state.analysis_active = false;
    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.analysis_state.lock), "Failed to initialize analysis state lock");
    FBK_ASSERT_MSG(0 == pthread_cond_init(&fbk_analysis_data.analysis_state.analysis_started_cond, NULL), "Failed to initialize analysis_started condition");

    /* Initialize job queue */
    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.job_queue.lock), "Failed to initialize job queue lock");
    FBK_ASSERT_MSG(0 == pthread_cond_init(&fbk_analysis_data.job_queue.new_job_available, NULL), "Failed to initialize job queue condition");
    FBK_ASSERT_MSG(0 == pthread_cond_init(&fbk_analysis_data.job_queue.job_claimed, NULL), "Failed to initialize job claimed condition");

    /* Initialize stats */
    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.analysis_stats.lock), "Failed to initialize analysis stats lock");

    /* Start manager thread */
    pthread_create(&fbk_analysis_data.worker_manager_thread, NULL, worker_manager_thread_f, &fbk_analysis_data);
  }
  else
  {
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Analysis data already initialized");
    ret_val = false;
  }

  return ret_val;
}

static void worker_thread_cleanup(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Cleaning up worker thread %u.", worker_thread_data->thread_index);
}

static void worker_thread_job_queue_cleanup(void *arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u releasing job queue mutex for thread cancellation.", worker_thread_data->thread_index);
  fbk_mutex_unlock(&worker_thread_data->job_queue->lock);
}


static void * worker_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;

  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u started with ID 0x%lx.", worker_thread_data->thread_index, pthread_self());

  pthread_cleanup_push(worker_thread_cleanup, worker_thread_data);

  while(1)
  {
    wait_for_analysis_start(worker_thread_data->analysis_state);

    /* Claim an analysis job */
    fbk_mutex_lock(&worker_thread_data->job_queue->lock);
    pthread_cleanup_push(worker_thread_job_queue_cleanup, worker_thread_data);
    while(worker_thread_data->job_queue->next_job == NULL)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread 0x%u waiting for job.", worker_thread_data->thread_index);
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&worker_thread_data->job_queue->new_job_available, &worker_thread_data->job_queue->lock),
        "Failed Waiting for new data available condition.");
    }

    /* Claim job and set pthread cancellation cleanup callback */

    pthread_cleanup_pop(0);
    fbk_mutex_unlock(&worker_thread_data->job_queue->lock);

    /* Process job */
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u processing job.", worker_thread_data->thread_index);
    sleep(1); /* Simulate job */
    
    /* Report job done or cancelled and remove pthread cancellation callback */
  }

  pthread_cleanup_pop(0);

  FBK_NO_RETURN
  return NULL;
}
/**
 * @brief Updates the configured number of worker threads
 * 
 * @param count new number of worker threads to allow
*/
void fbk_update_worker_thread_count(unsigned int count)
{
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Configuring %u worker threads.", count);

  if(count < 1)
  {
    FBK_ERROR_MSG("At least 1 worker thread is required but %u is requested.  Falling back to 1 worker thread", count);
    fbk_analysis_data.fbk->config.worker_threads = 1;
  }
  else
  {
    fbk_analysis_data.fbk->config.worker_threads = count;
  }

  if(fbk_analysis_data.fbk->config.worker_threads > fbk_analysis_data.worker_thread_count)
  {
    /* Spawn worker threads */

    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Allocating memory for new worker thread(s).");
    /* Allocate additional worker thread data pointers */
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s*)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker thread pointers failed.\n", fbk_analysis_data.fbk->config.worker_threads);

    for(unsigned int i = fbk_analysis_data.worker_thread_count; i < fbk_analysis_data.fbk->config.worker_threads; i++)
    {
      /* Configure each new entry */
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Creating worker thread %u.", i);
      /* Allocate thread data */
      fbk_analysis_data.worker_thread_data[i]=calloc(1, sizeof(fbk_worker_thread_data_s));
      FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data[i] != NULL, "Alloc for worker threads %u failed.\n", i);
      /* Configured thread data */
      fbk_analysis_data.worker_thread_data[i]->thread_index   = i;
      fbk_analysis_data.worker_thread_data[i]->job_queue      = &fbk_analysis_data.job_queue;
      fbk_analysis_data.worker_thread_data[i]->analysis_state = &fbk_analysis_data.analysis_state;
      FBK_ASSERT_MSG(0 == pthread_create(&fbk_analysis_data.worker_thread_data[i]->worker_thread, NULL, worker_thread_f, fbk_analysis_data.worker_thread_data[i]),
        "Failed to create worker thread %u", i);
    }
  }
  else if(fbk_analysis_data.fbk->config.worker_threads < fbk_analysis_data.worker_thread_count)
  {
    /* Kill worker threads */

    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Cancelling worker thread %u.", i);
      /* Trigger cancellation for all excess worker threads */
      FBK_ASSERT_MSG(0 == pthread_cancel(fbk_analysis_data.worker_thread_data[i]->worker_thread),
        "Failed to trigger cancel for worker thread %u", i);
    }
    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      /* Wait for all excess worker threads to finish cancellation */
      FBK_ASSERT_MSG(0 == pthread_join(fbk_analysis_data.worker_thread_data[i]->worker_thread, NULL), 
                    "Failed to join worker thread %u.", i);
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u cancelled.", i);
      free(fbk_analysis_data.worker_thread_data[i]);
    }
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Releasing memory for excess worker thread(s).");
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker threads failed.\n", fbk_analysis_data.fbk->config.worker_threads);
  }

  fbk_analysis_data.worker_thread_count = fbk_analysis_data.fbk->config.worker_threads;
}

void fbk_start_analysis(fbk_move_tree_node_s * node)
{
  fbk_mutex_lock(&fbk_analysis_data.analysis_state.lock);
  fbk_analysis_data.analysis_state.analysis_active = true;
  fbk_analysis_data.analysis_state.root_node       = node;
  pthread_cond_broadcast(&fbk_analysis_data.analysis_state.analysis_started_cond);
  fbk_mutex_unlock(&fbk_analysis_data.analysis_state.lock);
}

void fbk_stop_analysis()
{
  fbk_mutex_lock(&fbk_analysis_data.analysis_state.lock);
  fbk_analysis_data.analysis_state.analysis_active = false;
  fbk_analysis_data.analysis_state.root_node       = NULL;
  fbk_mutex_unlock(&fbk_analysis_data.analysis_state.lock);
}
/*
 fly_by_knight_analysis_worker.c
 Fly by Knight - Chess Engine
 Edward Sandor
 March 2023
 
 Gama analysis worker logic for Fly by Knight
*/

#include <string.h>

#include "fly_by_knight_analysis_worker.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"

fbk_analysis_data_s fbk_analysis_data = {0};

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

    /* Initialize stats */
    FBK_ASSERT_MSG(fbk_mutex_init(&fbk_analysis_data.analysis_stats.lock), "Failed to initialize analysis stats lock");
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
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Cleaning up worker thread %u.", worker_thread_data->thread_id);
}

static void worker_thread_job_queue_cleanup(void *arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u releasing job queue mutex for thread cancellation.", worker_thread_data->thread_id);
  fbk_mutex_unlock(&worker_thread_data->job_queue->lock);
}

static void worker_thread_analysis_state_cleanup(void *arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u releasing analysis state mutex for thread cancellation.", worker_thread_data->thread_id);
  fbk_mutex_unlock(&worker_thread_data->analysis_state->lock);
}


static void * worker_thread_f(void * arg)
{
  FBK_ASSERT_MSG(arg != NULL, "NULL worker thread data passed.");
  fbk_worker_thread_data_s *worker_thread_data = (fbk_worker_thread_data_s *) arg;

  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Worker thread %u started.", worker_thread_data->thread_id);

  pthread_cleanup_push(worker_thread_cleanup, worker_thread_data);

  while(1)
  {
    /* Check if analysis is active */
    fbk_mutex_lock(&worker_thread_data->analysis_state->lock);
    pthread_cleanup_push(worker_thread_analysis_state_cleanup, worker_thread_data);
    while(!worker_thread_data->analysis_state->analysis_active)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u waiting for analysis to start.", worker_thread_data->thread_id);
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&worker_thread_data->analysis_state->analysis_started_cond, &worker_thread_data->analysis_state->lock),
        "Failed Waiting for analysis started condition.");
    }
    pthread_cleanup_pop(0);
    fbk_mutex_unlock(&worker_thread_data->analysis_state->lock);

    /* Claim an analysis job */
    fbk_mutex_lock(&worker_thread_data->job_queue->lock);
    pthread_cleanup_push(worker_thread_job_queue_cleanup, worker_thread_data);
    while(worker_thread_data->job_queue->next_job == NULL)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u waiting for job.", worker_thread_data->thread_id);
      FBK_ASSERT_MSG(0 == pthread_cond_wait(&worker_thread_data->job_queue->new_job_available, &worker_thread_data->job_queue->lock),
        "Failed Waiting for new data available condition.");
    }
    pthread_setcancelstate(PTHREAD_CANCEL_DISABLE, NULL);
    pthread_cleanup_pop(0);

    /* Claim job and set pthread cancellation cleanup callback */
    fbk_mutex_unlock(&worker_thread_data->job_queue->lock);
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL);

    /* Process job */
    
    /* Report job done or cancelled and remove pthread cancellation callback */
  }

  pthread_cleanup_pop(0);
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
    /* Allocate additional worker thread data entries */
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker threads failed.\n", fbk_analysis_data.fbk->config.worker_threads);

    for(unsigned int i = fbk_analysis_data.worker_thread_count; i < fbk_analysis_data.fbk->config.worker_threads; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Creating worker thread %u.", i);
      /* Configure each new entry */
      memset(&fbk_analysis_data.worker_thread_data[i], 0, sizeof(fbk_worker_thread_data_s));
      fbk_analysis_data.worker_thread_data[i].thread_id      = i;
      fbk_analysis_data.worker_thread_data[i].job_queue      = &fbk_analysis_data.job_queue;
      fbk_analysis_data.worker_thread_data[i].analysis_state = &fbk_analysis_data.analysis_state;
      pthread_create(&fbk_analysis_data.worker_thread_data[i].worker_thread, NULL, worker_thread_f, &fbk_analysis_data.worker_thread_data[i]);
    }
  }
  else if(fbk_analysis_data.fbk->config.worker_threads < fbk_analysis_data.worker_thread_count)
  {
    /* Kill worker threads */

    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Cancelling worker thread %u.", i);
      /* Trigger cancellation for all excess worker threads */
      pthread_cancel(fbk_analysis_data.worker_thread_data[i].worker_thread);
    }
    for(unsigned int i = fbk_analysis_data.fbk->config.worker_threads; i < fbk_analysis_data.worker_thread_count; i++)
    {
      /* Wait for all excess worker threads to finish cancellation */
      FBK_ASSERT_MSG(0 == pthread_join(fbk_analysis_data.worker_thread_data[i].worker_thread, NULL), 
                    "Failed to join worker thread %u.", i);
      FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Worker thread %u cancelled.", i);
    }
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "Releasing memory for excess worker thread(s).");
    fbk_analysis_data.worker_thread_data = realloc(fbk_analysis_data.worker_thread_data, sizeof(fbk_worker_thread_data_s)*fbk_analysis_data.fbk->config.worker_threads);
    FBK_ASSERT_MSG(fbk_analysis_data.worker_thread_data != NULL, "Realloc for %u worker threads failed.\n", fbk_analysis_data.fbk->config.worker_threads);
  }

  fbk_analysis_data.worker_thread_count = fbk_analysis_data.fbk->config.worker_threads;
}
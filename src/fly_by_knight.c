/*
 fly_by_knight.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
 Main file for Fly by Knight
*/

#include <getopt.h>
#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <farewell_to_king.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight_analysis.h"
#include "fly_by_knight_analysis_worker.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_io.h"
#include "fly_by_knight_move_tree.h"
#include "fly_by_knight_pick.h"
#include "fly_by_knight_types.h"
#include "fly_by_knight_version.h"

/**
 * @brief Initialize Fly by Knight Mutex
 * 
 * @param mutex  Mutex to init
 * @return bool  True if successful
 */
bool fbk_mutex_init(fbk_mutex_t *mutex)
{
  bool ret_val = true; 

  if(mutex)
  {
    ret_val = (0 == pthread_mutex_init(mutex, NULL));
  }
  else
  {
    ret_val = false;
  }

  return ret_val;
}

/**
 * @brief Destroy Fly by Knight Mutex
 * 
 * @param mutex  Mutex to destroy
 * @return bool  True if successful
 */
bool fbk_mutex_destroy(fbk_mutex_t *mutex)
{
  bool ret_val = true; 

  if(mutex)
  {
    int rc = pthread_mutex_destroy(mutex);
    if(rc != 0)
    {
      FBK_ERROR_MSG_HARD("Error %d destroying mutex.", rc);
    }
    ret_val = (0 == rc);
  }
  else
  {
    ret_val = false;
  }

  return ret_val;
}

/**
 * @brief Locks Fly by Knight Mutex
 * 
 * @param mutex  Mutex to lock 
 * @return bool  True if successful
 */
bool fbk_mutex_lock(fbk_mutex_t *mutex)
{
  bool ret_val = true; 

  if(mutex)
  {
    int rc = pthread_mutex_lock(mutex);
    if(rc != 0)
    {
      FBK_ERROR_MSG_HARD("Error %d locking mutex.", rc);
    }
    ret_val = (0 == rc);
  }
  else
  {
    ret_val = false;
  }

  return ret_val;
}

/**
 * @brief Unlocks Fly by Knight Mutex
 * 
 * @param mutex  Mutex to unlock 
 * @return bool  True if successful
 */
bool fbk_mutex_unlock(fbk_mutex_t *mutex)
{
  bool ret_val = true; 

  if(mutex)
  {
    int rc = pthread_mutex_unlock(mutex);
    if(rc != 0)
    {
      FBK_ERROR_MSG_HARD("Error %d unlocking mutex.", rc);
    }
    ret_val = (0 == rc);
  }
  else
  {
    ret_val = false;
  }

  return ret_val;
}

/**
 * @brief Begins a new standard game.  Resets move tree and setups up game
 * 
 * @param fbk Fly by Knight context
 * @param flush_analysis flush analysis after setting board
 */
void fbk_begin_standard_game(fbk_instance_s * fbk, bool flush_analysis)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  if(fbk->move_tree.initialized)
  {
    fbk_stop_analysis(true);
    fbk_stop_picker();
    fbk->move_tree.initialized = false;
    if(flush_analysis)
    {
      fbk_delete_move_tree_node(&fbk->move_tree.root);
    }
  }

  ftk_begin_standard_game(&fbk->game);

  fbk_init_move_tree_node(&fbk->move_tree.root, NULL, NULL);
  fbk->move_tree.current = &fbk->move_tree.root;
  fbk->move_tree.initialized = true;

  /* Reset the analysis counter */
  reset_game_analyzed_nodes();
}

/**
 * @brief Commits move to game and updates move tree
 * 
 * @param fbk 
 * @param move 
 */
bool fbk_commit_move(fbk_instance_s * fbk, ftk_move_s * move)
{
  bool ret_val = true;
  fbk_move_tree_node_s *node;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");
  FBK_ASSERT_MSG(move != NULL, "NULL move pointer passed.");

  /* Evaluate this node if not evaluated to generate child nodes */
  fbk_evaluate_move_tree_node(fbk->move_tree.current, &fbk->game, false);

  /* Find node for given move */
  node = fbk_get_move_tree_node_for_move(fbk->move_tree.current, move);

  if(node)
  {
    /* Commit move */
    ftk_move_forward(&fbk->game, move);
    fbk->move_tree.current = node;
  }
  else
  {
    ret_val = false;
  }

  /* Reset the analysis counter */
  reset_analyzed_nodes();

  return ret_val;
}

/**
 * @brief Undoes move based on FBK move tree
 * 
 * @param fbk 
 * @return true if successful
 * @return false if cannot undo move
 */
bool fbk_undo_move(fbk_instance_s * fbk)
{
  bool ret_val = true;
  fbk_mutex_t * node_lock;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");
  FBK_ASSERT_MSG(fbk->move_tree.current != NULL, "NULL current move tree node.");

  node_lock = &fbk->move_tree.current->lock;

  FBK_ASSERT_MSG(true == fbk_mutex_lock(node_lock), "Failed to lock node mutex");

  if(fbk->move_tree.current->parent != NULL &&
     FTK_MOVE_VALID(fbk->move_tree.current->move))
  {
    ret_val = (FTK_SUCCESS == ftk_move_backward(&fbk->game, &fbk->move_tree.current->move));
    fbk->move_tree.current = fbk->move_tree.current->parent;
  }
  else
  {
    ret_val = false;
  }

  FBK_ASSERT_MSG(true == fbk_mutex_unlock(node_lock), "Failed to unlock node mutex");

  /* Reset the analysis counter */
  reset_analyzed_nodes();

  return ret_val;
}

/**
 * @brief Parsed argument data
 * 
 */
typedef struct 
{
  unsigned int worker_threads;
} fbk_arguments_s;

/**
 * @brief Initializes Fly by Knight
 * 
 * @param fbk       Fly by Knight instance data
 * @param arguments Arguments parsed from command line
 */
void init(fbk_instance_s * fbk, const fbk_arguments_s * arguments)
{
  FBK_ASSERT_MSG(fbk != NULL,       "NULL fbk_instance pointer passed.");
  FBK_ASSERT_MSG(arguments != NULL, "NULL arguments pointer passed.");
  FBK_DEBUG_MSG(FBK_DEBUG_MED,      "Initializing Fly by Knight");

  memset(fbk, 0, sizeof(fbk_instance_s));
  fbk->protocol = FBK_PROTOCOL_UNDEFINED;
  
  fbk->config.random           = false;
  fbk->config.analysis_breadth = FBK_DEFAULT_ANALYSIS_BREADTH;
  fbk->config.opponent_type    = FBK_OPPONENT_UNKNOWN;

  setbuf(stdout, NULL);

  fbk_begin_standard_game(fbk, true);

  FBK_ASSERT_MSG(fbk_init_analysis_lut(), "Failed to initialize analysis look-up tables");
  FBK_ASSERT_MSG(fbk_init_analysis_data(fbk), "Failed to initialize analysis data");
  fbk_update_worker_thread_count(arguments->worker_threads);

  fbk_init_picker(fbk);
}

/**
 * @brief Exits Fly by Knight cleanly and return code to calling process
 * 
 * @param return_code 
 */
void fbk_exit(int return_code)
{
  fbk_close_log_file(false);

  exit(return_code);
}

/**
 * @brief Display help text and exit if requested
 * 
 * @param user_requested true if user requested help text
 * @param exit_fbk true if program should exit
 */
void display_help(bool user_requested, bool exit_fbk)
{
  FILE * output_stream = (user_requested?stdout:stderr);
  fprintf(output_stream,
          "Usage: flybyknight [OPTION]...\n"
          "Chess engine following the xboard protocol with the UCI protocol in mind.\n"
          "  -d#,        --debug=#       start with debug logging level [0(disabled) - 9(maximum)]\n"
          "  -h,         --help          display this help and exit\n"
          "  -j#,        --jobs=#        start with given number of worker threads\n"
          "  -l [path],  --log=[path]    log output to file at given 'path'\n"
          "  -v,         --version       display complete version information\n");
  
  if(exit_fbk)
  {
    if(user_requested)
    {
      /* User requested, exit cleanly */
      fbk_exit(0);
    }
    else
    {
      /* Triggered by bad arguments, exit with error */
      fbk_exit(1);
    }
  }
}

/**
 * @brief Reports additional version details including supporting libraries
 * 
 */
void display_version_details(bool print_stdout)
{
  const char * version_str = ftk_get_intro_string();

  FBK_LOG_MSG(FLY_BY_KNIGHT_INTRO "\n");

  if(print_stdout)
  {
    /* Output and log*/
    FBK_OUTPUT_MSG("%s\n", version_str);
  }
  else
  {
    /* Always output and log if with debug */
    FBK_DEBUG_MSG(FBK_DEBUG_HIGH, "%s", version_str);
  }
}

void handle_signal(int signal)
{
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Received signal %u", signal);
  /* Clean exit with bash signal code */
  fbk_exit(128+signal);
}

void fbk_set_random_number_seed(unsigned int seed)
{
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Using random number seed %u", seed);
  srand(seed);
} 

/**
 * @brief Parses arguments passed with command
 * 
 * @param argc      argc from main()
 * @param argv      argv from main()
 * @param arguments Output structure of parsed arguments
 */
void parse_arguments(int argc, char *argv[], fbk_arguments_s *arguments)
{
  int i;
  bool version_details_requested = false;
  time_t curr_time;
  unsigned int random_seed;

  /* Seed random numbers */
  time(&curr_time);
  random_seed = curr_time;

  FBK_ASSERT_MSG(arguments != NULL, "Empty arguments structure passed");

  memset(arguments, 0, sizeof(fbk_arguments_s));
  arguments->worker_threads = 1;

  int option;
  int option_index = 0;
  static struct option long_options[] = {
      {"debug",   required_argument, 0,  'd' },
      {"jobs",    required_argument, 0,  'j' },
      {"log",     required_argument, 0,  'l' },
      {"help",    no_argument,       0,  'h' },
      {"version", no_argument,       0,  'v' },
      {0,         0,                 0,   0  }
  };

  bool argument_error = false;
  while(!argument_error && ((option = getopt_long(argc, argv, "d:j:l:hv", long_options, &option_index)) != -1))
  {
    switch(option)
    {
      case 'd':
      {
        int debug = atoi(optarg);
        if((debug > FBK_DEBUG_MIN) || (debug < FBK_DEBUG_DISABLED))
        {
          argument_error = true;
        }
        else
        {
          fbk_set_debug_level(debug);
        }
        break;
      }
      case 'j':
      {
        arguments->worker_threads = atoi(optarg);
        if(arguments->worker_threads < 1)
        {
          FBK_ERROR_MSG("At least 1 worker thread is required but argument passed %u.", arguments->worker_threads);
          argument_error = true;
        }
        break;
      }
      case 'l':
      {
        fbk_open_log_file(optarg);
        break;
      }
      case 'h':
      {
        display_help(true, true);
        break;
      }
      case 'v':
      {
        version_details_requested = true;
        break;
      }
      default:
      {
        argument_error = true;
        break;
      }
    }
  }
  if(argument_error)
  {
    display_help(false, true);
  }

  // Log command and arguments
  FBK_LOG_MSG("# [COMMAND]: ");
  for(i = 0; i < argc; i++)
  {
    FBK_LOG_MSG("%s ", argv[i]);
  }
  FBK_LOG_MSG("\n");

  /* Log version details */
  display_version_details(version_details_requested);
  /* Set random number seed */
  fbk_set_random_number_seed(random_seed);
}

fbk_instance_s fbk_instance;
int main(int argc, char *argv[])
{
  /* Introduce Fly by Knight */
  printf(FLY_BY_KNIGHT_INTRO "\n");

  /* Configure signal handlers */
  signal(SIGINT,  handle_signal);
  signal(SIGTERM, handle_signal);

  /* Parse command arguments */
  fbk_arguments_s arguments;
  parse_arguments(argc, argv, &arguments);

  /* Initialize Fly by Knight root structure */
  init(&fbk_instance, &arguments);

  /* Start IO handler on main thread, analysis to be done on separate threads */
  fly_by_knight_io_thread(&fbk_instance);

  return 0;
}
/*
 fly_by_knight.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
 Main file for Fly by Knight
*/

#include <pthread.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

#include <farewell_to_king.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_io.h"
#include "fly_by_knight_move_tree.h"
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
    ret_val = (0 == pthread_mutex_destroy(mutex));
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
    ret_val = (0 == pthread_mutex_lock(mutex));
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
    ret_val = (0 == pthread_mutex_unlock(mutex));
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
 * @param fbk 
 */
void fbk_begin_standard_game(fbk_instance_s * fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  /* TODO stop and flush analysis */

  fbk_delete_move_tree_node(&fbk->move_tree.root);

  ftk_begin_standard_game(&fbk->game);

  fbk_init_move_tree_node(&fbk->move_tree.root, NULL, NULL);
  fbk->move_tree.current = &fbk->move_tree.root;
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

  /* Evalute this node if not evaluated to generate child nodes */
  fbk_evaluate_move_tree_node(fbk->move_tree.current, &fbk->game);

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

  return ret_val;
}

/**
 * @brief Initializes Fly by Knight
 * 
 * @param fbk Fly by Knight instance data
 * @param debug true if debug logging should be enabled
 */
void init(fbk_instance_s * fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Initializing Fly by Knight");

  memset(fbk, 0, sizeof(fbk_instance_s));
  fbk->protocol = FBK_PROTOCOL_UNDEFINED;
  
  fbk->config.random           = false;
  fbk->config.max_search_depth = FBK_DEFAULT_MAX_SEARCH_DEPTH;
  fbk->config.opponent_type    = FBK_OPPONENT_UNKNOWN;

  fbk_begin_standard_game(fbk);
}

/**
 * @brief Exits Fly by Knight cleanly and return code to calling process
 * 
 * @param return_code 
 */
void fbk_exit(int return_code)
{
  fbk_close_log_file();

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
  printf( "Usage: fly_by_knight [OPTION]...\n"
          "Chess engine following the UCI protocol\n"
          "  -d#, --debug=#  start with debug logging level [0(disabled) - 9(maximum)]\n"
          "  -h, --help      display this help and exit\n");
  
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
 * @brief Parsed argument data
 * 
 */
typedef struct 
{
  /* Argument params as needed */
  void * placeholder;
} fbk_arguments_s;

/**
 * @brief Parses arguments passed with command
 * 
 * @param argc      argc from main()
 * @param argv      argv from main()
 * @param arguments Output structure of parsed arguments
 */
void parse_arguments(int argc, const char ** argv, fbk_arguments_s *arguments)
{
  int i;
  fbk_debug_level_t debug = FBK_DEBUG_DISABLED;
  bool version_details_requested = false;
  time_t curr_time;
  unsigned int random_seed;

  /* Seed random numbers */
  time(&curr_time);
  random_seed = curr_time;

  FBK_ASSERT_MSG(arguments != NULL, "Empty arguments structure passed");

  memset(arguments, 0, sizeof(fbk_arguments_s));

  for(i = 1; i < argc; i++)
  {
    if(strncmp(argv[i], "-d", 2) == 0 || strncmp(argv[i], "--debug=", 8) == 0)
    {
      if(strncmp(argv[i], "-d", 2) == 0)
      {
        debug = argv[i][2] - '0';
      }
      else if(strncmp(argv[i], "--debug=", 8) == 0)
      {
        debug = argv[i][8] - '0';
      }

      if(debug > 9)
      {
        display_help(false, true);
      }
      else
      {
        fbk_set_debug_level(debug);
      }
    }
    else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0)
    {
      if(i + 1 < argc)
      {
        i++;
        fbk_open_log_file(argv[i]);
      }
      else
      {
        display_help(false, true);
      }
    }
    else if(strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0)
    {
      display_help(true, true);
    }
    else if(strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0)
    {
      version_details_requested = true;
    }
    else
    {
      display_help(false, true);
    }
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

int main(int argc, char ** argv)
{
  fbk_arguments_s arguments;
  fbk_instance_s fbk_instance;

  /* Introduce Fly by Knight */
  printf(FLY_BY_KNIGHT_INTRO "\n");

  /* Configure signal handlers */
  signal(SIGINT,  SIG_IGN);
  signal(SIGTERM, handle_signal);

  /* Parse command arguments */
  parse_arguments(argc, (const char **) argv, &arguments);

  /* Initialize Fly by Knight root structure */
  init(&fbk_instance);

  /*
  int presult;
  pthread_t io_thread;
  presult = pthread_create(&io_thread, NULL, fly_by_knight_io_thread, &fbk_instance);
  FBK_ASSERT_MSG(0 == presult, "Failed to start IO thread, presult %d", presult);
  */

  /* Start IO handler on main thread, analysis to be done on separate threads */
  fly_by_knight_io_thread(&fbk_instance);

  return 0;
}
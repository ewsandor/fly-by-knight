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
#include "fly_by_knight_types.h"
#include "fly_by_knight_version.h"

fbk_instance_s fbk_instance;

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

  ftk_begin_standard_game(&fbk->game);
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
 * @brief Displays addtional version details including supporting libraries
 * 
 */
void display_version_details(bool print_stdout)
{
  char * version_str = FAREWELL_TO_KING_INTRO;

  if(print_stdout)
  {
    printf("%s\n", version_str);
  }

  FBK_DEBUG_MSG(FBK_DEBUG_LOW, "%s", version_str);
}

void handle_signal(int signal)
{
  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Received signal %u", signal);
  /* Clean exit with bash signal code */
  fbk_exit(128+signal);
}

int main(int argc, char ** argv)
{
  printf(FLY_BY_KNIGHT_INTRO "\n");

  signal(SIGINT,  SIG_IGN);
  signal(SIGTERM, handle_signal);

  fbk_debug_level_t debug = false;
  bool print_version = false;
  char * log_path = NULL;
  uint i;
  int presult;
  pthread_t io_thread;
  time_t curr_time;

  
  srand((unsigned) time(&curr_time));

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
        log_path = argv[i+1];
        i++;
        fbk_open_log_file(log_path);
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
      print_version = true;
    }
    else
    {
      display_help(false, true);
    }
  }

  display_version_details(print_version);

  init(&fbk_instance);

  // Log command and arguments
  FBK_LOG_MSG("# [COMMAND]: ");
  for(i = 0; i < argc; i++)
  {
    FBK_LOG_MSG("%s ", argv[i]);
  }
  FBK_LOG_MSG("\n");

  presult = pthread_create(&io_thread, NULL, fly_by_knight_io_thread, &fbk_instance);
  FBK_ASSERT_MSG(0 == presult, "Failed to start IO thread, presult %d", presult);

  pause();

  /*
  ftk_game_s game;
  ftk_game_end_e game_end;
  ftk_begin_standard_game(&game);

  char input[128];
  char out[1024];
  ftk_move_s move = {0};

  for(;;){
    //printBoard(game.board, out);
    ftk_board_to_string_with_coordinates(&game.board, out);
    printf("%s\r\n", out);

    game_end = ftk_check_for_game_end(&game);
    if(FTK_END_NOT_OVER == game_end)
    {
      if(FTK_CHECK_IN_CHECK == ftk_check_for_check(&game))
      {
        printf("CHECK!\r\n\r\n");
      }
    }
    else if (FTK_END_CHECKMATE == game_end) 
    {
      printf("CHECKMATE!\r\n\r\n");
    }
    else if (FTK_END_DRAW_STALEMATE == game_end) 
    {
      printf("STALEMATE!\r\n\r\n");
    }
    else {
      printf("GAME OVER! Reason %u\r\n\r\n", game_end);
    }

    ftk_game_to_fen_string(&game, out);
    printf("%s\r\n", out);

    scanf("%s", input);

    if(strcmp("q",input) == 0 || strcmp("quit",input)==0)
        break;

    if(strcmp("u",input) == 0)
    {
      ftk_move_backward(&game, &move);
    }
    else if(strcmp("r",input) == 0)
    {
      ftk_move_forward(&game, &move);
    }
    else if(strcmp("n",input) == 0)
    {
      ftk_begin_standard_game(&game);
    }
    else 
    {
      ftk_position_t target = FTK_XX;
      ftk_position_t source = FTK_XX;

      ftk_type_e   pawn_promo_type = FTK_TYPE_EMPTY;
      ftk_castle_e castle_type     = FTK_CASTLE_NONE;
      ftk_xboard_move(input, &target, &source, &pawn_promo_type, &castle_type);

      move = ftk_move_piece(&game, target, source, pawn_promo_type);

      ftk_move_backward(&game, &move);
      printf("%d\n\n", ftk_move_forward(&game, &move));
    }
  }
  */

  return 0;
}
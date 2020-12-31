/*
 fly_by_knight.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Main file for Fly by Knight
*/

#include <pthread.h>

#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#include <farewell_to_king_strings.h>

#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_io.h"
#include "fly_by_knight_types.h"
#include "fly_by_knight_version.h"

/**
 * @brief Initializes Fly by Knight
 * 
 * @param fbk Fly by Knight instance data
 * @param debug true if debug logging should be enabled
 */
void init(fbk_instance_s * fbk, fbk_debug_level_t debug_level, char * log_path)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  memset(fbk, 0, sizeof(fbk_instance_s));
  fbk->debug_level = debug_level;

  fbk_open_log_file(fbk, log_path);

  FBK_DEBUG_MSG(*fbk, FBK_DEBUG_MED, "Initializing Fly by Knight");

  fbk->protocol   = FBK_PROTOCOL_UNDEFINED;
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
      exit(0);
    }
    else
    {
      /* Triggered by bad arguments, exit with error */
      exit(1);
    }
  }
}

void handle_signal(int signal)
{
  /* Clean exit with bash signal code */
  exit(128 + signal);
}

int main(int argc, char ** argv)
{
  printf(FLY_BY_KNIGHT_INTRO "\n");

  signal(SIGINT, handle_signal);

  fbk_debug_level_t debug = false;
  char * log_path = NULL;
  uint i;
  int presult;
  pthread_t io_thread;
  fbk_instance_s fbk_instance;

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
    }
    else if(strcmp(argv[i], "-l") == 0 || strcmp(argv[i], "--log") == 0)
    {
      if(i + 1 < argc)
      {
        log_path = argv[i+1];
        i++;
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
    else
    {
      display_help(false, true);
    }
  }

  init(&fbk_instance, debug, log_path);

  // Log command and arguments
  FBK_LOG_MSG(fbk_instance, "# [COMMAND]: ");
  for(i = 0; i < argc; i++)
  {
    FBK_LOG_MSG(fbk_instance, "%s ", argv[i]);
  }
  FBK_LOG_MSG(fbk_instance, "\n");

  presult = pthread_create(&io_thread, NULL, fly_by_knight_io_thread, &fbk_instance);
  FBK_ASSERT_LOG(fbk_instance, 0 == presult, "Failed to start IO thread, presult %d", presult);

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
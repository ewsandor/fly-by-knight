/*
 fly_by_knight.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Main file for Fly by Knight
*/

#include <pthread.h>

#include <stdio.h>
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
void init(fbk_instance_s * fbk, bool debug)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  memset(fbk, 0, sizeof(fbk_instance_s));
  fbk->debug_mode = debug;

  FBK_DEBUG_MSG(*fbk, "Initializing Fly by Knight");

  fbk->protocol   = FBK_PROTOCOL_UNDEFINED;
}

int main()
{
  printf(FLY_BY_KNIGHT_NAME " version " FLY_BY_KNIGHT_VERSION_STR " by " FLY_BY_KNIGHT_AUTHOR " <" FLY_BY_KNIGHT_CONTACT ">\n");

  int presult;
  pthread_t io_thread;
  fbk_instance_s fbk_instance;

  init(&fbk_instance, true);

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
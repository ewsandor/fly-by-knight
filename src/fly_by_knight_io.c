/*
 fly_by_knight_io.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Input and Output handling for Fly by Knight
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <farewell_to_king.h>
#include <farewell_to_king_board.h>
#include <farewell_to_king_types.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_io.h"
#include "fly_by_knight_types.h"
#include "fly_by_knight_uci.h"
#include "fly_by_knight_version.h"

#define FBK_INPUT_BUFFER_SIZE  1024
#define FBK_OUTPUT_BUFFER_SIZE 1024

void *fly_by_knight_io_thread(void *fbk_instance)
{
  fbk_instance_s *fbk = (fbk_instance_s *) fbk_instance;

  bool input_handled;
  void * fgets_result;
  char input_buffer[FBK_INPUT_BUFFER_SIZE];
  char output_buffer[FBK_INPUT_BUFFER_SIZE];
  size_t strlength;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  while(true)
  {
    // Read from stdin
    memset(input_buffer, 0, sizeof(input_buffer));
    fgets_result = fgets(input_buffer, FBK_INPUT_BUFFER_SIZE, stdin);
    FBK_ASSERT_MSG((fgets_result == input_buffer), "Failed to read from stdin");

    // Strip trailing newline
    strlength = strlen(input_buffer);
    if(strlength > 0)
    {
      FBK_ASSERT_MSG(strlength < FBK_INPUT_BUFFER_SIZE, "Invalid string");

      if('\n' == input_buffer[strlength-1])
      {
        input_buffer[strlength-1] = '\0';
      }
    }
    
    // Handle input
    input_handled = true;
    if(strcmp("quit", input_buffer) == 0)
    {
      break;
    }
    else if(strcmp("print", input_buffer) == 0)
    {
      ftk_board_to_string_with_coordinates(&fbk->game.board, output_buffer);
      printf("%s\n", output_buffer);
    }
    else if(strcmp("print fen", input_buffer) == 0)
    {
      ftk_game_to_fen_string(&fbk->game, output_buffer);
      printf("%s\n", output_buffer);
    }
    else if(FBK_PROTOCOL_UNDEFINED == fbk->protocol)
    {
      if(strcmp("uci", input_buffer) == 0)
      {
        fbk->protocol = FBK_PROTOCOL_UCI;

        /* Acknowledge UCI mode */
        printf("id name " FLY_BY_KNIGHT_NAME_VER "\n"
               "id author " FLY_BY_KNIGHT_AUTHOR "\n"
               /* TODO specify options */
               "uciok\n"); 
      }
      else if(strcmp("xboard", input_buffer) == 0)
      {
        FBK_ERROR_MSG("XBoard Protocol not supported");
      }
      else
      {
        input_handled = false;
      }
    }
    else if(FBK_PROTOCOL_UCI == fbk->protocol)
    {
      input_handled = fbk_process_uci_input(fbk, input_buffer);
    }
    else
    {
      FBK_FATAL_MSG("Unsupported protocol %d", fbk->protocol);
    }

    if(false == input_handled) 
    {
      printf("Error (unknown command): %s\n", input_buffer);
    }
  }

  exit(0);
}
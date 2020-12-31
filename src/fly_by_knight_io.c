/*
 fly_by_knight_io.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Input and Output handling for Fly by Knight
*/

#include <errno.h>
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


/**
 * @brief Open file for logging
 * 
 * @param fbk 
 * @param log_path 
 */
void fbk_open_log_file(fbk_instance_s * fbk, char * log_path)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  if(log_path != NULL)
  {
    FBK_DEBUG_MSG(*fbk, FBK_DEBUG_MED, "Opening %s for logging", log_path);
    
    FBK_ASSERT_LOG(*fbk, NULL == fbk->log_file, "Log handle already open.");

    fbk->log_file = fopen(log_path, "a");

    if(NULL == fbk->log_file)
    {
      FBK_FATAL_MSG("Failed to open %s for logging (errno %d)", log_path, errno);
    }

    FBK_LOG_MSG(*fbk, FLY_BY_KNIGHT_INTRO "\n");
  }
}

/**
 * @brief Close log file
 * 
 * @param fbk 
 */
void fbk_close_log_file(fbk_instance_s * fbk)
{
  int result;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk_instance pointer passed.");

  if(fbk->log_file != NULL)
  {
    FBK_DEBUG_MSG(*fbk, FBK_DEBUG_MED, "Closing log file");

    result = fclose(fbk->log_file);

    FBK_ASSERT_LOG(*fbk, 0 == result, "Failed to close log file (errno %d)", errno);
  }
}

/**
 * @brief Thread for handling IO
 * 
 * @param fbk_instance 
 * @return void* 
 */
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
      FBK_ASSERT_LOG(*fbk, strlength < FBK_INPUT_BUFFER_SIZE, "Invalid string");

      if('\n' == input_buffer[strlength-1])
      {
        input_buffer[strlength-1] = '\0';
      }
    }

    FBK_LOG_MSG(*fbk, "# [INPUT]: %s\n", input_buffer);
    
    // Handle input
    input_handled = true;
    if(strcmp("quit", input_buffer) == 0)
    {
      break;
    }
    else if(strcmp("print", input_buffer) == 0)
    {
      ftk_board_to_string_with_coordinates(&fbk->game.board, output_buffer);
      FBK_OUTPUT_MSG(*fbk, "%s\n", output_buffer);
    }
    else if(strcmp("print fen", input_buffer) == 0)
    {
      ftk_game_to_fen_string(&fbk->game, output_buffer);
      FBK_OUTPUT_MSG(*fbk, "%s\n", output_buffer);
    }
    else if(FBK_PROTOCOL_UNDEFINED == fbk->protocol)
    {
      if(strcmp("uci", input_buffer) == 0)
      {
        fbk->protocol = FBK_PROTOCOL_UCI;

        /* Acknowledge UCI mode */
        FBK_OUTPUT_MSG(*fbk, "id name " FLY_BY_KNIGHT_NAME_VER "\n"
                       "id author " FLY_BY_KNIGHT_AUTHOR "\n"
                       /* TODO specify options */
                       "uciok\n");
      }
      else if(strcmp("xboard", input_buffer) == 0)
      {
        FBK_ERROR_MSG("XBoard Protocol not supported");
        input_handled = false;
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
      FBK_FATAL_LOG(*fbk, "Unsupported protocol %d", fbk->protocol);
    }

    if(false == input_handled) 
    {
      FBK_OUTPUT_MSG(*fbk, "Error (unknown command): %s\n", input_buffer);
    }
  }

  exit(0);
}
/*
 fly_by_knight_io.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
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
#include "fly_by_knight_xboard.h"

#define FBK_INPUT_BUFFER_SIZE  1024
#define FBK_OUTPUT_BUFFER_SIZE 1024

//Logging file if enabled
bool fbk_log_file_configured = false;
FILE *fbk_log_file = NULL;

/**
 * @brief Open file for logging
 * 
 * @param log_path 
 */
void fbk_open_log_file(char * log_path)
{
  if(log_path != NULL)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_MED, "Opening %s for logging", log_path);
    
    FBK_ASSERT_MSG(NULL == fbk_log_file, "Log handle already open.");

    fbk_log_file = fopen(log_path, "a");

    if(NULL == fbk_log_file)
    {
      FBK_FATAL_MSG("Failed to open %s for logging (errno %d)", log_path, errno);
    }

    fbk_log_file_configured = true;

    FBK_LOG_MSG(FLY_BY_KNIGHT_INTRO "\n");
  }
}

/**
 * @brief Close log file
 */
void fbk_close_log_file()
{
  int result;

  if(fbk_log_file != NULL)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_MED, "Closing log file");

    fbk_log_file_configured = false;

    result = fclose(fbk_log_file);

    FBK_ASSERT_MSG(0 == result, "Failed to close log file (errno %d)", errno);
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
    // Flush pending output
    fflush(stdout);

    // Read from stdin
    memset(input_buffer, 0, sizeof(input_buffer));
    fgets_result = fgets(input_buffer, FBK_INPUT_BUFFER_SIZE, stdin);

    if(fgets_result != input_buffer)
    {
      if(feof(stdin))
      {
        FBK_DEBUG_MSG(FBK_DEBUG_LOW, "STDIN EOF.  Exiting");
        break;
      }
      else
      {
        FBK_FATAL_MSG("Failed to read from stdin (ferror %d)", ferror(stdin));
      }
    }


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

    FBK_LOG_MSG("# [INPUT]: %s\n", input_buffer);
    
    // Handle input
    input_handled = true;
    if(strcmp("quit", input_buffer) == 0)
    {
      break;
    }
    else if(strcmp("print", input_buffer) == 0)
    {
      ftk_board_to_string_with_coordinates(&fbk->game.board, output_buffer);
      FBK_OUTPUT_MSG("%s\n", output_buffer);
    }
    else if(strcmp("print fen", input_buffer) == 0)
    {
      ftk_game_to_fen_string(&fbk->game, output_buffer);
      FBK_OUTPUT_MSG("%s\n", output_buffer);
    }
    else if(FBK_PROTOCOL_UNDEFINED == fbk->protocol)
    {
      if(strcmp("uci", input_buffer) == 0)
      {
        FBK_FATAL_MSG("UCI not fully supported yet, please prefer xboard");

        fbk->protocol = FBK_PROTOCOL_UCI;

        /* Acknowledge UCI mode */
        FBK_OUTPUT_MSG("id name " FLY_BY_KNIGHT_NAME_VER "\n"
                       "id author " FLY_BY_KNIGHT_AUTHOR "\n"
                       /* TODO specify options */
                       "uciok\n");
      }
      else if(strcmp("xboard", input_buffer) == 0)
      {
        fbk_init_xboard_protocol(fbk);
      }
      else
      {
        input_handled = false;
      }
    }
    else if(FBK_PROTOCOL_XBOARD == fbk->protocol)
    {
      input_handled = fbk_process_xboard_input(fbk, input_buffer);
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
      FBK_OUTPUT_MSG("Error (unknown command): %s\n", input_buffer);
    }
  }

  fbk_exit(0);

  return NULL;
}
/*
 fly_by_knight_uci.c
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020 - 2021
 
 UCI protocol interpetting for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_uci.h"

/**
 * @brief Process position command
 * 
 * @param fbk Fly by Knight instance data
 * @param input Input string from external process
 */
void fbk_process_uci_position_command(fbk_instance_s *fbk, char * input)
{
  ftk_result_e ftk_result;

  FBK_DEBUG_MSG(FBK_DEBUG_MED, "Processing position command: %s", input);

  if(strncmp("fen", input, 3) == 0)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "FEN position received: %s", &input[4]);
    ftk_result = ftk_create_game_from_fen_string(&fbk->game, &input[4]);
    FBK_ASSERT_MSG(FTK_SUCCESS == ftk_result, "Failed to parse FEN string: %s (%u)", &input[4], ftk_result);
  }
  else if(strncmp("startpos moves", input, 14) == 0)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_LOW, "startpos received: %s", &input[15]);
    fbk_begin_standard_game(fbk);

    char move_string[FTK_MOVE_STRING_SIZE] = {0};
    ftk_square_e target, source;
    ftk_type_e     pawn_promotion;
    ftk_castle_e   castle;
    ftk_move_s     move;

    for(unsigned int i = 15; true; i++)
    {
      unsigned int move_string_idx = 0;
      if(input[i] != ' ' && input[i] != '\0')
      {
        FBK_ASSERT_MSG(move_string_idx < FTK_MOVE_STRING_SIZE, "move string exceeded max size: %s", move_string);
        move_string[move_string_idx] = input[i];
        move_string_idx++;
      }
      else if(move_string_idx > 0)
      {
        FBK_DEBUG_MSG(FBK_DEBUG_MIN, "processing move: %s", move_string);

        target = FTK_XX;
        source = FTK_XX;
        pawn_promotion = FTK_TYPE_EMPTY;
        castle         = FTK_CASTLE_NONE;
        ftk_result = ftk_long_algebraic_move(move_string, &target, &source, &pawn_promotion, &castle);          
        FBK_ASSERT_MSG(FTK_SUCCESS == ftk_result, "Failed to parse move string: %s", move_string);

        move = ftk_move_piece(&fbk->game, target, source, pawn_promotion);
        FBK_ASSERT_MSG(FTK_XX != move.source, "Illegal move %s", move_string);
        FBK_ASSERT_MSG(FTK_XX != move.target, "Illegal move %s", move_string);

        memset(move_string, 0, sizeof(move_string));
        move_string_idx = 0;
      }
      if('\0' == input[i])
      {
        break;
      }
    }
  }
  else
  {
    FBK_FATAL_MSG("Cannot process position command: %s", input);
  }
}

/**
 * @brief Process input string with UCI protocol
 * 
 * @param fbk Fly by Knight instance data
 * @param input Input string from external process
 * @return true if input handled
 * @return false if input is not UCI
 */
bool fbk_process_uci_input(fbk_instance_s *fbk, char * input)
{
  bool input_handled = true;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk pointer passed.");
  FBK_ASSERT_MSG(input != NULL, "NULL input pointer passed.");

  if(strcmp("isready", input) == 0)
  {
    FBK_OUTPUT_MSG("readyok\n");
  }
  else if(strncmp("position", input, 8) == 0)
  {
    fbk_process_uci_position_command(fbk, &input[9]);
  }
  else if(strcmp("stop", input) == 0)
  {
    //TODO stop ongoing analysis, reset decision maker
    //TODO return best move and best response (ponder) from decision maker
    FBK_OUTPUT_MSG("bestmove 0000\n");
  }
  else if(strcmp("ucinewgame", input) == 0)
  {
    //TODO stop ongoing analysis, reset decision maker, flush analysis
    fbk_begin_standard_game(fbk);
  }
  else if(strcmp("go", input) == 0)
  {
    //TODO start analysis and decision maker based on GUI's instructions
  }
  else if(strcmp("debug on", input) == 0)
  {
    fbk_set_debug_level(FBK_DEBUG_HIGH);
    FBK_DEBUG_MSG(FBK_DEBUG_HIGH, "debug logging enabled (level %u)", FBK_DEBUG_HIGH);
  }
  else if(strcmp("debug off", input) == 0)
  {
    fbk_set_debug_level(FBK_DEBUG_DISABLED);
  }
  else
  {
    input_handled = false;
  }

  return input_handled;
}
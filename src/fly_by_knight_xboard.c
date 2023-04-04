/*
 fly_by_knight_xboard.c
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 xboard protocol interpreting for Fly by Knight
*/

#include <string.h>

#include <farewell_to_king.h>
#include <farewell_to_king_strings.h>

#include "fly_by_knight_analysis_worker.h"
#include "fly_by_knight_debug.h"
#include "fly_by_knight_error.h"
#include "fly_by_knight_pick.h"
#include "fly_by_knight_version.h"
#include "fly_by_knight_xboard.h"

/**
 * @brief Initialized FBK for xboard
 * 
 * @param fbk 
 */
void fbk_init_xboard_protocol(fbk_instance_s *fbk)
{
  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk pointer passed.");

  fbk->protocol = FBK_PROTOCOL_XBOARD;

  memset(&fbk->protocol_data, 0, sizeof(fbk_protocol_data_u));

  fbk->protocol_data.xboard.version = 1;
  fbk->protocol_data.xboard.mode    = FBK_XBOARD_MODE_NORMAL; 
}

/**
 * @brief Configure feature options with the GUI 
 * 
 * @param fbk 
 */
void fbk_xboard_config_features(fbk_instance_s *fbk)
{
  FBK_UNUSED(fbk);

  FBK_OUTPUT_MSG("feature done=0\n"
                 "feature ping=1\n"
                 "feature setboard=1\n"
                 "feature playother=1\n"
                 "feature san=0\n"
                 "feature usermove=0\n"
                 "feature time=0\n"
                 "feature draw=0\n"
                 "feature sigint=0\n"
                 "feature sigterm=0\n"
                 "feature reuse=1\n"
                 "feature analyze=0\n"
                 "feature myname=\"" FLY_BY_KNIGHT_NAME_VER "\"\n"
                 "feature variants=\"normal\"\n"
                 "feature colors=0\n"
                 "feature ics=0\n"
                 "feature name=1\n"
                 "feature pause=0\n"
                 "feature nps=0\n"
                 "feature debug=1\n"
                 "feature memory=0\n"
                 "feature smp=1\n"
               //"feature egt=null\n"
               //"feature option=null\n"
                 "feature exclude=0\n"
                 "feature setscore=0\n"
                 "feature highlight=0\n"
                 "feature done=1\n"
                );
}

/**
 * @brief Handle rejected features
 * 
 * @param fbk 
 * @param input 
 */
void fbk_xboard_handle_rejected_feature(fbk_instance_s *fbk, char * input)
{
  FBK_UNUSED(fbk);

  if(strcmp("debug", input) == 0)
  {
    fbk_set_debug_level(FBK_DEBUG_DISABLED);
  }
}

/**
 * @brief Handles input string when xboard is in normal mode
 * 
 * @param fbk 
 * @param input 
 * @param input_length 
 * @return true 
 * @return false 
 */
bool fbk_process_xboard_input_normal_mode(fbk_instance_s *fbk, char * input, size_t input_length)
{
  bool input_handled = true;
  ftk_result_e ftk_result;

  if(strncmp("protover", input, 8) == 0)
  {
    xboard_version_t version;
    if(input_length >= 10)
    {
      version = input[9] - '0';
      if(version <= 9)
      {
        FBK_DEBUG_MSG(FBK_DEBUG_MED, "Using xboard protocol version %u", version);
        fbk->protocol_data.xboard.version = version;
        if(fbk->protocol_data.xboard.version > 1)
        {
          fbk_xboard_config_features(fbk);
        }
      }
      else
      {
        FBK_ERROR_MSG("Error parsing xboard protocol");
        input_handled = false;
      }
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strncmp("accepted", input, 8) == 0)
  {
    if(input_length > 9)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_MED, "Feature '%s' accepted", &input[9]);
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strncmp("rejected", input, 8) == 0)
  {
    if(input_length > 9)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_MED, "Feature '%s' rejected", &input[9]);
      fbk_xboard_handle_rejected_feature(fbk, &input[9]);
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strcmp("force", input) == 0)
  {
    fbk_stop_analysis(true);
    fbk->protocol_data.xboard.play_as = FTK_COLOR_NONE;
    fbk->protocol_data.xboard.ponder  = false;
  }
  else if(strcmp("go", input) == 0)
  {
    fbk->protocol_data.xboard.play_as = fbk->game.turn;
  }
  else if(strcmp("playother", input) == 0)
  {
   fbk->protocol_data.xboard.play_as = (FTK_COLOR_WHITE == fbk->game.turn)?FTK_COLOR_BLACK:FTK_COLOR_WHITE;
  }
  /* Disable 'color' support for now, deprecated in xboard protocol 2 and requires the move tree to be rebuilt if engine is playing non-standard rules (e.g. black goes first)
  else if(strcmp("white", input) == 0)
  {
    //stop clock
    fbk->protocol_data.xboard.play_as = FTK_COLOR_WHITE;
    fbk->game.turn                    = FTK_COLOR_BLACK;
  }
  else if(strcmp("black", input) == 0)
  {
    //stop clock
    fbk->protocol_data.xboard.play_as = FTK_COLOR_BLACK;
    fbk->game.turn                    = FTK_COLOR_WHITE;
  }*/
  else if(strcmp("?", input) == 0)
  {
    //Force decision maker
  }
    else if(strcmp("random", input) == 0)
  {
    fbk->config.random = !fbk->config.random;
  }
  else if(strcmp("post", input) == 0)
  {
    fbk->config.analysis_output = true;
  }
  else if(strcmp("nopost", input) == 0)
  {
    fbk->config.analysis_output = false;
  }
  else if(strcmp("easy", input) == 0)
  {
    fbk_stop_analysis(true);
    fbk->protocol_data.xboard.ponder = false;
  }
  else if(strcmp("hard", input) == 0)
  {
    fbk->protocol_data.xboard.ponder = true;
  }
  else if(strncmp("cores", input, 5) == 0)
  {
    if(input_length > 6)
    {
      fbk_update_worker_thread_count(atoi(&input[6]));
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strncmp("name", input, 4) == 0)
  {
    if(input_length > 5)
    {
      FBK_DEBUG_MSG(FBK_DEBUG_HIGH, "Opponent - %s", &input[5]);
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strcmp("edit", input) == 0)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_HIGH, "Entering xboard edit mode");
    fbk->protocol_data.xboard.mode = FBK_XBOARD_MODE_EDIT;
    fbk->protocol_data.xboard.edit_color = FTK_COLOR_WHITE;
  }
  else if(strcmp("new", input) == 0)
  {
    fbk_begin_standard_game(fbk);

    fbk->protocol_data.xboard.play_as = FTK_COLOR_BLACK;
    fbk->config.max_search_depth      = FBK_DEFAULT_MAX_SEARCH_DEPTH;
  }
  else if(strncmp("ping", input, 4) == 0)
  {
    if(input_length > 5)
    {
      FBK_OUTPUT_MSG("pong %s\n", &input[5]);
    }
    else
    {
      FBK_DEBUG_MSG(FBK_DEBUG_MED, "ping received without argument");
      FBK_OUTPUT_MSG("pong\n");
    }
  }
  else if(strncmp("setboard", input, 8) == 0)
  {
    /* TODO stop analysis and flush */
    if(input_length > 9)
    {
      fbk_begin_standard_game(fbk);
      ftk_result = ftk_create_game_from_fen_string(&fbk->game, &input[9]);
      FBK_ASSERT_MSG(FTK_SUCCESS == ftk_result, "Failed to parse FEN string: %s (%u)", &input[9], ftk_result);
    }
    else
    {
      FBK_OUTPUT_MSG("Error (too few parameters): %s\n", input);
    }
  }
  else if(strncmp("computer", input, 8) == 0)
  {
    fbk->config.opponent_type = FBK_OPPONENT_COMPUTER;
  }
  else if(strncmp("undo", input, 4) == 0)
  {
    FBK_ASSERT_MSG(true == fbk_undo_move(fbk), "Failed to undo move");
  }
  else if(strncmp("remove", input, 6) == 0)
  {
    FBK_ASSERT_MSG(true == fbk_undo_move(fbk), "Failed to undo move");
    FBK_ASSERT_MSG(true == fbk_undo_move(fbk), "Failed to undo move");
  }
  else
  {
    unsigned int i, move_string_idx = 0;
    char move_string[FTK_MOVE_STRING_SIZE] = {0};
    ftk_square_e   target, source;
    ftk_type_e     pawn_promotion;
    ftk_castle_e   castle;
    ftk_move_s     move;

    input_handled = false;

    for(i = 0; move_string_idx < FTK_MOVE_STRING_SIZE; i++)
    {
      if(input[i] != ' ' && input[i] != '\0')
      {
        move_string[move_string_idx] = input[i];
        move_string_idx++;
      }
      else if(move_string_idx > 0)
      {

        target = FTK_XX;
        source = FTK_XX;
        pawn_promotion = FTK_TYPE_EMPTY;
        castle         = FTK_CASTLE_NONE;
        ftk_result = ftk_xboard_move(move_string, &target, &source, &pawn_promotion, &castle);          

        if(FTK_SUCCESS == ftk_result && FTK_XX != target && FTK_XX != source)
        {
          FBK_DEBUG_MSG(FBK_DEBUG_LOW, "processing move: %s", move_string);
          input_handled = true;
          move = ftk_stage_move(&fbk->game, target, source, pawn_promotion);

          if(FTK_MOVE_VALID(move))
          {
            FBK_ASSERT_MSG(true == fbk_commit_move(fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
          }
          else
          {
            FBK_OUTPUT_MSG("Illegal move: %s\n", move_string);
          }
        }
        break;
      }
    }
  }

  return input_handled;
}

/**
 * @brief Handles input string when xboard is in edit mode
 * 
 * @param fbk 
 * @param input 
 * @return true 
 * @return false 
 */
bool fbk_process_xboard_input_edit_mode(fbk_instance_s *fbk, char * input, size_t input_length)
{
  bool input_handled = true;

  if(strcmp(".", input) == 0)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_HIGH, "Exiting xboard edit mode");
    fbk->protocol_data.xboard.mode = FBK_XBOARD_MODE_NORMAL;
    ftk_update_board_masks(&fbk->game);
  }
  else if(strcmp("c", input) == 0)
  {
    fbk->protocol_data.xboard.edit_color = (FTK_COLOR_WHITE == fbk->protocol_data.xboard.edit_color)?FTK_COLOR_BLACK:FTK_COLOR_WHITE;
    FBK_DEBUG_MSG(FBK_DEBUG_MED, "Editing %s pieces", (FTK_COLOR_WHITE == fbk->protocol_data.xboard.edit_color)?"WHITE":"BLACK");
  }
  else if(strcmp("#", input) == 0)
  {
    FBK_DEBUG_MSG(FBK_DEBUG_MED, "Clearing board");
    ftk_clear_board(&fbk->game.board);
  }
  else if(3 == input_length)
  {
    ftk_square_e mod_square = ftk_string_to_position(&input[1]);
    
    if(mod_square != FTK_XX)
    {
      if(input[0] == 'x' || input[0] == 'X')
      {
        FTK_SQUARE_CLEAR(fbk->game.board.square[mod_square]);
      }
      else
      {
        ftk_moved_status_e piece_status = FTK_MOVED_HAS_MOVED;
        ftk_type_e piece_type = ftk_char_to_piece_type(input[0]);

        if(piece_type != FTK_TYPE_EMPTY)
        {
          if( (FTK_TYPE_KING == piece_type && FTK_COLOR_WHITE == fbk->protocol_data.xboard.edit_color && FTK_E1 == mod_square) ||
              (FTK_TYPE_ROOK == piece_type && FTK_COLOR_WHITE == fbk->protocol_data.xboard.edit_color && FTK_A1 == mod_square) ||
              (FTK_TYPE_ROOK == piece_type && FTK_COLOR_WHITE == fbk->protocol_data.xboard.edit_color && FTK_H1 == mod_square) ||
              (FTK_TYPE_KING == piece_type && FTK_COLOR_BLACK == fbk->protocol_data.xboard.edit_color && FTK_E8 == mod_square) || (FTK_TYPE_ROOK == piece_type && FTK_COLOR_BLACK == fbk->protocol_data.xboard.edit_color && FTK_A8 == mod_square) ||
              (FTK_TYPE_ROOK == piece_type && FTK_COLOR_BLACK == fbk->protocol_data.xboard.edit_color && FTK_H8 == mod_square) )
          {
            piece_status = FTK_MOVED_NOT_MOVED;
          }

          FTK_SQUARE_SET(fbk->game.board.square[mod_square], piece_type, fbk->protocol_data.xboard.edit_color, piece_status);
        }
        else
        {
          FBK_OUTPUT_MSG("Error (ambiguous edit): %s", input);
        }
      }
    }
    else
    {
      FBK_OUTPUT_MSG("Error (ambiguous edit): %s", input);
    }
  }
  else
  {
    input_handled = false;
  }
  
  return input_handled;
}

void manage_xboard_analysis(fbk_instance_s * fbk)
{
  if( (FBK_XBOARD_MODE_NORMAL == fbk->protocol_data.xboard.mode) &&
      ((fbk->game.turn == fbk->protocol_data.xboard.play_as) || fbk->protocol_data.xboard.ponder))
  {
    fbk_start_analysis(&fbk->game, fbk->move_tree.current);
  }
  else
  {
    fbk_stop_analysis(false);
  }
}

/**
 * @brief Process input string with xboard protocol
 * 
 * @param fbk Fly by Knight instance data
 * @param input Input string from external process
 * @return true if input handled
 * @return false if input is not xboard
 */
bool fbk_process_xboard_input(fbk_instance_s *fbk, char * input)
{
  bool input_handled = true;
  size_t input_length = strlen(input);
  ftk_game_end_e game_result;

  FBK_ASSERT_MSG(fbk != NULL, "NULL fbk pointer passed.");
  FBK_ASSERT_MSG(input != NULL, "NULL input pointer passed.");

  if(FBK_XBOARD_MODE_NORMAL == fbk->protocol_data.xboard.mode)
  {
    input_handled = fbk_process_xboard_input_normal_mode(fbk, input, input_length);
  }
  else if(FBK_XBOARD_MODE_EDIT == fbk->protocol_data.xboard.mode)
  {
    input_handled = fbk_process_xboard_input_edit_mode(fbk, input, input_length);
  }
  else
  {
    FBK_FATAL_MSG("Unsupported xboard mode %u", fbk->protocol_data.xboard.mode);
  }

  manage_xboard_analysis(fbk);

  game_result = ftk_check_for_game_end(&fbk->game);
  if(FTK_END_NOT_OVER == game_result)
  {
    fbk->protocol_data.xboard.result_reported = false;

    if(FBK_XBOARD_MODE_NORMAL ==fbk->protocol_data.xboard.mode)
    {
      /* Temporary logic to return simple best move - Replace with periodic decision logic based on analysis depth and clocks */
      if(fbk->protocol_data.xboard.play_as == fbk->game.turn)
      {
        ftk_move_s move;
        /* Null move by default */
        char move_output[FTK_MOVE_STRING_SIZE] = "@@@@";

        move = fbk_get_best_move(fbk);

        if(FTK_MOVE_VALID(move))
        {
          /* Commit move */
          FBK_ASSERT_MSG(true == fbk_commit_move(fbk, &move), "Failed to commit move (%u->%u)", move.source, move.target);
          ftk_move_to_xboard_string(&move, move_output);
        }

        FBK_OUTPUT_MSG("move %s\n", move_output);

        manage_xboard_analysis(fbk);
      }
    }
  }
  else if(false == fbk->protocol_data.xboard.result_reported)
  {
    if(FTK_END_DEFINITIVE(game_result))
    {
      if(FTK_COLOR_BLACK == fbk->game.turn)
      {
        FBK_OUTPUT_MSG("1-0 {White mates}\n");
      }
      else
      {
        FBK_OUTPUT_MSG("0-1 {Black mates}\n");
      }
    }
    else
    {
      FBK_ASSERT_MSG(FTK_END_DRAW(game_result), "Unexpected game result %u", game_result);

      if(FTK_END_DRAW_STALEMATE == game_result)
      {
        FBK_OUTPUT_MSG("1/2-1/2 {Stalemate}\n");
      }
      else
      {
        FBK_OUTPUT_MSG("1/2-1/2 {Rule %u}\n", game_result);
      }
    }

    fbk->protocol_data.xboard.result_reported = true;
  }


  return input_handled;
}
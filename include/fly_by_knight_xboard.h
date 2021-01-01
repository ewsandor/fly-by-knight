/*
 fly_by_knight_xboard.h
 Fly by Knight - Chess Engine
 Edward Sandor
 January 2021
 
 xboard protocol interpetting for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_XBOARD_H_
#define _FLY_BY_KNIGHT_XBOARD_H_

#include "fly_by_knight_types.h"

/**
 * @brief Initialized FBK for xboard
 * 
 * @param fbk 
 */
void fbk_init_xboard_protocol(fbk_instance_s *fbk);

/**
 * @brief Process input string with xboard protocol
 * 
 * @param fbk Fly by Knight instance data
 * @param input Input string from external process
 * @return true if input handled
 * @return false if input is not xboard
 */
bool fbk_process_xboard_input(fbk_instance_s *fbk, char * input);

#endif //_FLY_BY_KNIGHT_XBOARD_H_
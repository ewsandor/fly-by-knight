/*
 fly_by_knight_uci.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 UCI protocol interpetting for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_UCI_H_
#define _FLY_BY_KNIGHT_UCI_H_

#include "fly_by_knight_types.h"

/**
 * @brief Process input string with UCI protocol
 * 
 * @param fbk Fly by Knight instance data
 * @param input Input string from external process
 * @return true if input handled
 * @return false if input is not UCI
 */
bool fbk_process_uci_input(fbk_instance_s *fbk, char * input);

#endif //_FLY_BY_KNIGHT_UCI_H_
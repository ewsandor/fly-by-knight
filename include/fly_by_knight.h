/*
 fly_by_knight.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common definitions for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_H_
#define _FLY_BY_KNIGHT_H_

#include "fly_by_knight_types.h"

/* Acknowledge variable is unused */
#define FBK_UNUSED(x) {(void)(x);}

/**
 * @brief Exits Fly by Knight cleanly and return code to calling process
 * 
 * @param return_code 
 */
void fbk_exit(int return_code);

/**
 * @brief Initialize Fly by Knight Mutex
 * 
 * @param mutex  Mutex to init
 * @return bool  True if successful
 */
bool fbk_mutex_init(fbk_mutex_t *mutex);

/**
 * @brief Destroy Fly by Knight Mutex
 * 
 * @param mutex  Mutex to destroy
 * @return bool  True if successful
 */
bool fbk_mutex_destroy(fbk_mutex_t *mutex);

/**
 * @brief Locks Fly by Knight Mutex
 * 
 * @param mutex  Mutex to lock 
 * @return bool  True if successful
 */
bool fbk_mutex_lock(fbk_mutex_t *mutex);

/**
 * @brief Unlocks Fly by Knight Mutex
 * 
 * @param mutex  Mutex to unlock 
 * @return bool  True if successful
 */
bool fbk_mutex_unlock(fbk_mutex_t *mutex);

/**
 * @brief Begins a new standard game.  Resets move tree and setups up game
 * 
 * @param fbk 
 */
void fbk_begin_standard_game(fbk_instance_s * fbk, bool flush_analysis);

/**
 * @brief Commits move to game and updates move tree
 * 
 * @param fbk 
 * @param move 
 */
bool fbk_commit_move(fbk_instance_s * fbk, ftk_move_s * move);

/**
 * @brief Undoes move based on FBK move tree
 * 
 * @param fbk 
 * @return true if successful
 * @return false if cannot undo move
 */
bool fbk_undo_move(fbk_instance_s * fbk);

#endif //_FLY_BY_KNIGHT_H_
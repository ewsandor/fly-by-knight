/*
 fly_by_knight_io.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common error checking and reporting for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_IO_H_
#define _FLY_BY_KNIGHT_IO_H_

#include <stdio.h>

#include "fly_by_knight_types.h"

extern bool fbk_log_file_configured;
extern FILE *fbk_log_file;

#define FBK_LOG_MSG(msg, ...) if(fbk_log_file_configured){fprintf(fbk_log_file, msg, ##__VA_ARGS__);}
#define FBK_OUTPUT_MSG(msg, ...) printf(msg, ##__VA_ARGS__); FBK_LOG_MSG(msg, ##__VA_ARGS__)

/**
 * @brief Open file for logging
 * 
 * @param log_path 
 */
void fbk_open_log_file(const char * log_path);

/**
 * @brief Close log file
 */
void fbk_close_log_file();

/**
 * @brief Thread for handling IO
 * 
 * @param fbk_instance 
 * @return void* 
 */
void *fly_by_knight_io_thread(void * fbk_instance);

#endif //_FLY_BY_KNIGHT_IO_H_
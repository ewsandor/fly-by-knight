/*
 fly_by_knight_error.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common error checking and reporting for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_ERROR_H_
#define _FLY_BY_KNIGHT_ERROR_H_

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>

#include "fly_by_knight.h"
#include "fly_by_knight_io.h"

#ifdef FBK_DEBUG_BUILD
#define FBK_DEBUG_EXIT
#endif

#define FBK_ERROR_MSG(msg, ...)             fprintf(stderr, "# [ERROR] FBK: "         msg "\n",                     ##__VA_ARGS__); FBK_LOG_MSG("# [ERROR] FBK: "         msg "\n",                     ##__VA_ARGS__)
#ifdef FBK_DEBUG_EXIT
#define FBK_ERROR_MSG_HARD(msg, ...)        fprintf(stderr, "# [ERROR] (%s:%d) FBK: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__); FBK_LOG_MSG("# [ERROR] (%s:%d) FBK: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define FBK_ASSERT_MSG(exp, msg, ...)       if(!(exp)) {FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__); fbk_exit(128+SIGABRT); }
#define FBK_FATAL_MSG_CODE(code, msg, ...)              FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__); fbk_exit(code);
#else
#define FBK_ERROR_MSG_HARD(msg, ...)        FBK_ERROR_MSG(msg, ##__VA_ARGS__);
#define FBK_ASSERT_MSG(exp, msg, ...)       if(!(exp)) {FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__);}
#define FBK_FATAL_MSG_CODE(code, msg, ...)              FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__);
#endif

#define FBK_FATAL_MSG(msg, ...)             FBK_FATAL_MSG_CODE((128+SIGABRT), msg, ##__VA_ARGS__)

#endif //_FLY_BY_KNIGHT_ERROR_H_
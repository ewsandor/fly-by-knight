/*
 fly_by_knight_error.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Common error checking and reporting for Fly by Knight
*/

#ifndef _FLY_BY_KNIGHT_ERROR_H_
#define _FLY_BY_KNIGHT_ERROR_H_

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

#include "fly_by_knight.h"
#include "fly_by_knight_io.h"

/* Error handlers when logging is not available (before initialized), output to stderr only */
#define FBK_ERROR_MSG(msg, ...) fprintf(stderr, "# [ERROR] FBK: " msg "\n", ##__VA_ARGS__)
#define FBK_ERROR_MSG_HARD(msg, ...) fprintf(stderr, "# [ERROR] (%s:%d) FBK: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define FBK_ASSERT_MSG(exp, msg, ...) if(!(exp)) {FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__); assert(exp); }
#define FBK_FATAL_MSG(msg, ...) FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__); exit(1);

/* Primary error handlers, output to file and stderr */
#define FBK_ERROR_LOG(fbk, msg, ...) FBK_ERROR_MSG(msg, ##__VA_ARGS__); FBK_LOG_MSG(fbk, "# [ERROR] FBK: " msg "\n", ##__VA_ARGS__)
#define FBK_ERROR_LOG_HARD(fbk, msg, ...) FBK_ERROR_MSG_HARD(msg, ##__VA_ARGS__); FBK_LOG_MSG(fbk, "# [ERROR] (%s:%d) FBK: " msg "\n", __FILE__, __LINE__, ##__VA_ARGS__)
#define FBK_ASSERT_LOG(fbk, exp, msg, ...) if(!(exp)) { FBK_ERROR_LOG_HARD(fbk, msg, ##__VA_ARGS__); fbk_close_log_file(&(fbk)); assert(exp); }
#define FBK_FATAL_LOG_CODE(fbk, code, msg, ...) FBK_ERROR_LOG_HARD(fbk, msg, ##__VA_ARGS__); fbk_exit(&(fbk), code);
#define FBK_FATAL_LOG(fbk, msg, ...) FBK_FATAL_LOG_CODE(fbk, 1, msg, ##__VA_ARGS__)


#endif //_FLY_BY_KNIGHT_ERROR_H_
/*
 fly_by_knight_version.h
 Fly by Knight - Chess Engine
 Edward Sandor
 December 2020
 
 Defines Fly by Knight version
*/

#ifndef _FLY_BY_KNIGHT_VERSION_H_
#define _FLY_BY_KNIGHT_VERSION_H_

#define FLY_BY_KNIGHT_VERSION_DEV 1

#define FLY_BY_KNIGHT_VERSION_STR_BASE "1.0.0-dev"

#ifdef FBK_DEBUG_BUILD
#define FLY_BY_KNIGHT_VERSION_STR FLY_BY_KNIGHT_VERSION_STR_BASE " <debug "__DATE__ " " __TIME__">"
#else
#define FLY_BY_KNIGHT_VERSION_STR FLY_BY_KNIGHT_VERSION_STR_BASE
#endif

#define FLY_BY_KNIGHT_NAME "Fly by Knight"
#define FLY_BY_KNIGHT_NAME_VER "Fly by Knight "FLY_BY_KNIGHT_VERSION_STR
#define FLY_BY_KNIGHT_AUTHOR "Edward Sandor"
#define FLY_BY_KNIGHT_CONTACT "flybyknight@sandorlaboratories.com"

#define FLY_BY_KNIGHT_INTRO FLY_BY_KNIGHT_NAME " version " FLY_BY_KNIGHT_VERSION_STR " by " FLY_BY_KNIGHT_AUTHOR " <" FLY_BY_KNIGHT_CONTACT ">"

#endif //_FLY_BY_KNIGHT_VERSION_H_
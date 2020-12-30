cmake_minimum_required (VERSION 3.13.4)
project (fly_by_knight)

option (BUILD_TEST "Build executable to test Fly by Knight." ON)

subdirs(src)
set(HEADERS "${CMAKE_SOURCE_DIR}/include")
include_directories(${HEADERS})

find_library(FAREWELL_TO_KING_LIB         farewell_to_king)
find_library(FAREWELL_TO_KING_STRINGS_LIB farewell_to_king_strings)
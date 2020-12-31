cmake_minimum_required (VERSION 3.13.4)
project (fly_by_knight)

option (BUILD_DEBUG "Build Fly by Knight with debug symbols enabled." ON)

if(BUILD_DEBUG)
  set(CMAKE_BUILD_TYPE Debug)
endif(BUILD_DEBUG)

subdirs(src)
set(HEADERS "${CMAKE_SOURCE_DIR}/include")
include_directories(${HEADERS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
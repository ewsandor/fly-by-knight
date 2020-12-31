cmake_minimum_required (VERSION 3.13.4)
project (fly_by_knight)

option (BUILD_TEST "Build executable to test Fly by Knight." ON)

subdirs(src)
set(HEADERS "${CMAKE_SOURCE_DIR}/include")
include_directories(${HEADERS})

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
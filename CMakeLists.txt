cmake_minimum_required (VERSION 3.26)
project (fly_by_knight)

option (BUILD_FTK_SHARED "ON to link Fly by Knight with shared Farewell to King library, else link statically." OFF)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFBK_DEBUG_BUILD")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(fly_by_knight  src/fly_by_knight.c 
                              src/fly_by_knight_analysis.c
                              src/fly_by_knight_debug.c
                              src/fly_by_knight_io.c
                              src/fly_by_knight_move_tree.c
                              src/fly_by_knight_pick.c
                              src/fly_by_knight_uci.c
                              src/fly_by_knight_xboard.c)

target_include_directories(fly_by_knight PRIVATE include)

add_subdirectory(lib/farewell-to-king EXCLUDE_FROM_ALL)
if(BUILD_FTK_SHARED)
  target_link_libraries(fly_by_knight PRIVATE farewell_to_king_shared)
else()
  target_link_libraries(fly_by_knight PRIVATE farewell_to_king)
endif()

find_package(Threads REQUIRED)
target_link_libraries(fly_by_knight PRIVATE Threads::Threads)
cmake_minimum_required (VERSION 3.18)
project (flybyknight)

option (BUILD_FTK_SHARED "ON to link Fly by Knight with shared Farewell to King library, else link statically." OFF)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DFBK_DEBUG_BUILD")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(flybyknight  src/fly_by_knight.c 
                              src/fly_by_knight_analysis.c
                              src/fly_by_knight_debug.c
                              src/fly_by_knight_io.c
                              src/fly_by_knight_move_tree.c
                              src/fly_by_knight_pick.c
                              src/fly_by_knight_uci.c
                              src/fly_by_knight_xboard.c)

target_include_directories(flybyknight PRIVATE include)

add_subdirectory(lib/farewell-to-king EXCLUDE_FROM_ALL)
if(BUILD_FTK_SHARED)
  target_link_libraries(flybyknight PRIVATE farewelltoking_shared)
else()
  target_link_libraries(flybyknight PRIVATE farewelltoking)
endif()

find_package(Threads REQUIRED)
target_link_libraries(flybyknight PRIVATE Threads::Threads)

install(TARGETS flybyknight)
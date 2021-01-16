cmake_minimum_required (VERSION 3.13.4)

add_executable (flybyknight fly_by_knight.c fly_by_knight_analysis.c fly_by_knight_debug.c fly_by_knight_io.c fly_by_knight_move_tree.c fly_by_knight_pick.c fly_by_knight_uci.c fly_by_knight_xboard.c)
target_link_libraries(flybyknight Threads::Threads)

if (BUILD_FTK_SHARED)
  target_link_libraries(flybyknight farewell_to_king_strings_shared farewell_to_king_shared)
else ()
  target_link_libraries(flybyknight farewell_to_king_strings_static farewell_to_king_static)
endif ()

install (TARGETS flybyknight DESTINATION bin)
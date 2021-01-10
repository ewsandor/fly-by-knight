cmake_minimum_required (VERSION 3.13.4)

add_executable (flybyknight fly_by_knight.c fly_by_knight_analysis.c fly_by_knight_debug.c fly_by_knight_io.c fly_by_knight_pick.c fly_by_knight_uci.c fly_by_knight_xboard.c)
target_link_libraries(flybyknight Threads::Threads farewell_to_king_strings farewell_to_king)
install (TARGETS flybyknight DESTINATION bin)
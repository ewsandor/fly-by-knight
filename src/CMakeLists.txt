cmake_minimum_required (VERSION 3.13.4)

add_executable (fly_by_knight fly_by_knight.c fly_by_knight_io.c)
target_link_libraries(fly_by_knight Threads::Threads farewell_to_king_strings farewell_to_king)
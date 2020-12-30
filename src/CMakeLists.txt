cmake_minimum_required (VERSION 3.13.4)

add_executable (fly_by_knight fly_by_knight.c)
target_link_libraries(fly_by_knight "${FAREWELL_TO_KING_LIB}")
target_link_libraries(fly_by_knight "${FAREWELL_TO_KING_STRINGS_LIB}")
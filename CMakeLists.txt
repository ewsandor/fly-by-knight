cmake_minimum_required (VERSION 3.18)
project (flybyknight)

option (ANALYSIS_NODE_COMPRESSION "ON to compress analysis nodes in RAM.  Turning this feature off removes the zlib dependency and may help debugging" ON)
option (BUILD_FTK_SHARED "ON to link Fly by Knight with shared Farewell to King library, else link statically." OFF)

set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DFBK_DEBUG_BUILD")

# lots of warnings and all warnings as errors
add_compile_options(-Wall -Wextra -pedantic -Werror)

if(MINGW)
# MinGW breaks many printf functions due to int size differences.  Disable these warning for now..."
message(WARNING "MINGW Detected: Disabling C string format warnings...")
add_compile_options(-Wformat=0)
endif()

add_executable(flybyknight  src/fly_by_knight.c 
                              src/fly_by_knight_analysis.c
                              src/fly_by_knight_analysis_worker.c
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

if(ANALYSIS_NODE_COMPRESSION)
  set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DFBK_ZLIB_COMPRESSION")
  find_package(ZLIB REQUIRED)
  target_link_libraries(flybyknight PRIVATE ZLIB::ZLIB)
endif()

install(TARGETS flybyknight)
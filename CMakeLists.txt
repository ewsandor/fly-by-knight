cmake_minimum_required (VERSION 3.18)
project (flybyknight)

option (ANALYSIS_NODE_COMPRESSION "ON to compress analysis nodes in RAM.  Turning this feature off removes the zlib dependency and may help debugging" ON)
option (BUILD_AS_LEGACY "ON to build Fly by Knight as a legacy version with appropriate suffix.  OFF to build as main version with no suffix." OFF)
option (BUILD_FTK_SHARED "ON to link Fly by Knight with shared Farewell to King library, else link statically." OFF)
option (XBOARD_PROTOCOL_SUPPORT "ON to build Fly by Knight with support for the xboard chess communication protocol.  OFF to build without this support." ON)
option (UCI_PROTOCOL_SUPPORT "ON to build Fly by Knight with support for the UCI chess communication protocol.  OFF to build without this support." ON)



set(CMAKE_C_FLAGS_DEBUG "${CMAKE_C_FLAGS_DEBUG} -DFBK_DEBUG_BUILD")

# lots of warnings and all warnings as errors
add_compile_options(-Wall -Wextra -pedantic -Werror)

if(MINGW)
# MinGW breaks many printf functions due to int size differences.  Disable these warning for now..."
message(WARNING "MINGW Detected: Disabling C string format warning errors...")
add_compile_options(-Wno-error=format)
endif()

add_executable(flybyknight  src/fly_by_knight.c 
                            src/fly_by_knight_analysis.c
                            src/fly_by_knight_analysis_worker.c
                            src/fly_by_knight_debug.c
                            src/fly_by_knight_io.c
                            src/fly_by_knight_move_tree.c
                            src/fly_by_knight_pick.c)


if(XBOARD_PROTOCOL_SUPPORT)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFBK_XBOARD_PROTOCOL_SUPPORT")
  target_sources(flybyknight PRIVATE src/fly_by_knight_xboard.c)
endif()
if(UCI_PROTOCOL_SUPPORT)
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DFBK_UCI_PROTOCOL_SUPPORT")
  target_sources(flybyknight PRIVATE src/fly_by_knight_uci.c)
endif()


if(BUILD_AS_LEGACY)
  set_target_properties(flybyknight PROPERTIES OUTPUT_NAME "flybyknight1")
endif()

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
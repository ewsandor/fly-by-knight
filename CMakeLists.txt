cmake_minimum_required (VERSION 3.18)
project (flybyknight)

option (BUILD_AS_LEGACY "ON to build Fly by Knight as a legacy version with appropriate suffix.  OFF to build as main version with no suffix." ON)

set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -DFBK_DEBUG_BUILD")

if (MSVC)
    # warning level 4 and all warnings as errors
    add_compile_options(/W4 /WX)
else()
    # lots of warnings and all warnings as errors
    add_compile_options(-Wall -Wextra -pedantic)
    #add_compile_options(-Wall -Wextra -pedantic -Werror)
endif()

add_executable(flybyknight board.cpp game.cpp main.cpp moveTree.cpp piece.cpp)
if(BUILD_AS_LEGACY)
set_target_properties(flybyknight PROPERTIES OUTPUT_NAME "flybyknight0")
endif()

find_package(Threads REQUIRED)
target_link_libraries(flybyknight PRIVATE Threads::Threads)

install(TARGETS flybyknight)
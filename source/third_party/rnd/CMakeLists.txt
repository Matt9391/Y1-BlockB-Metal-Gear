cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

project(rnd)

SET(RND_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(rnd INTERFACE)

target_sources(rnd PUBLIC ${RND_INCLUDE_DIRECTORY}/rnd/rnd.h)

target_include_directories(rnd INTERFACE ${RND_INCLUDE_DIRECTORY})

set_target_properties(rnd PROPERTIES
                      FOLDER "third-party")

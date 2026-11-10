cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

project(args)

SET(ARGS_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include)

add_library(args INTERFACE)

target_sources(args PUBLIC ${ARGS_INCLUDE_DIRECTORY}/args/args.h)

target_include_directories(args INTERFACE ${ARGS_INCLUDE_DIRECTORY})

set_target_properties(args PROPERTIES
                      FOLDER "third-party")

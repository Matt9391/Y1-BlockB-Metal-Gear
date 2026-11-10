cmake_minimum_required(VERSION 3.0...3.20 FATAL_ERROR)

project(yesno)

set(YESNO_INCLUDE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/include)
set(YESNO_SOURCE_DIRECTORY  ${CMAKE_CURRENT_LIST_DIR}/src)

add_library(yesno STATIC)

target_sources(yesno PRIVATE
    ${YESNO_SOURCE_DIRECTORY}/yes_no.cpp
)

target_sources(yesno PUBLIC
    ${YESNO_INCLUDE_DIRECTORY}/yes_no.h
)

target_include_directories(yesno PUBLIC ${YESNO_INCLUDE_DIRECTORY})

set_target_properties(yesno PROPERTIES
                      FOLDER "third-party")

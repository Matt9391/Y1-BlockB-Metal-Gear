# Every folder in here with a CMakeLists.txt is a game, and is added
# automatically: to start a new game, copy the sample_2d folder, rename it
# (and the target name inside its CMakeLists.txt), and run generate again.
FILE(GLOB game_folders LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
LIST(SORT game_folders)
FOREACH(game_folder ${game_folders})
    IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${game_folder}/CMakeLists.txt)
        ADD_SUBDIRECTORY(${game_folder})
    ENDIF()
ENDFOREACH()

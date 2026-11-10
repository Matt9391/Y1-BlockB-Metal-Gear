# Every folder in here with a CMakeLists.txt is an engine module (see
# cmake/buz_module.cmake). They are picked up automatically, which is also
# what lets a student template leave out the modules of a higher tier.
FILE(GLOB module_folders LIST_DIRECTORIES true RELATIVE ${CMAKE_CURRENT_LIST_DIR} ${CMAKE_CURRENT_LIST_DIR}/*)
LIST(SORT module_folders)
FOREACH(module_folder ${module_folders})
    IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/${module_folder}/CMakeLists.txt)
        ADD_SUBDIRECTORY(${module_folder})
    ENDIF()
ENDFOREACH()

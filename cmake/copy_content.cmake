# -------------------------------
# Copy the content folder (textures, ...) next to the executable so the
# engine can load assets with paths relative to the executable. Defined
# before the games are added, so each game's own CMakeLists.txt can
# depend on it directly with ADD_DEPENDENCIES(<target> copy-content).
# -------------------------------
FILE(GLOB_RECURSE CONTENT_FILES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/content/*")
SOURCE_GROUP(TREE "${CMAKE_SOURCE_DIR}/content" PREFIX "content" FILES ${CONTENT_FILES})

ADD_CUSTOM_TARGET(copy-content ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_SOURCE_DIR}/content" "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content"
    COMMENT "Copying \"${CMAKE_SOURCE_DIR}/content\" folder to \"${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content\""
    SOURCES ${CONTENT_FILES}
    VERBATIM)

SET_PROPERTY(TARGET copy-content PROPERTY FOLDER "events")

# -------------------------------
# BUZ_GAME: one game, one executable.
#
# Every game lives in its own folder under source/games, and its
# CMakeLists.txt is a single call:
#
#   BUZ_GAME(sample_2d TIER tmpl_2d
#            LINKS buz_engine)
#
# LINKS are the engine modules the game uses: buz_engine for every game,
# plus the 3D modules for a 3D one (see the 3D sample's CMakeLists.txt).
#
# What it sets up, so no game has to repeat it:
# - Sources: every .cpp/.h/.hpp file under the game's private/ folder. You
#   never edit CMakeLists.txt to add code: create the new file inside
#   source/games/<game>/private (NOT inside the build/ folder!) and build.
#   CMake notices it and Visual Studio asks to reload.
# - The shaders in /shaders are compiled, and the shared /content folder is
#   copied next to the executable.
# - A game with a content/ folder of its own gets that copied too.
# - Every build warns about source files accidentally created in build/.
# - Package targets for the IDE, in its "package" folder: build
#   package_<game> to package the game for this OS, package_<game>_web for
#   the browser (OpenGL builds only). The selected configuration picks the
#   package: Release is the shipping one, Development the one that logs.
#   They do what the package scripts do (cmake/package.cmake).
#
# There is no VS_DEBUGGER_WORKING_DIRECTORY on purpose: the engine finds its
# content next to the executable (buz::paths::get_executable_directory), so
# the working directory makes no difference. Setting it to a generator
# expression also broke opening the repository as a folder in Visual Studio,
# which passes the property on unexpanded.
# -------------------------------
FUNCTION(BUZ_GAME name)
    CMAKE_PARSE_ARGUMENTS(GAME "" "TIER" "LINKS" ${ARGN})
    IF(NOT GAME_TIER)
        MESSAGE(FATAL_ERROR "BUZ_GAME(${name}) needs a TIER")
    ENDIF()

    MESSAGE(STATUS "Adding game - ${name}")
    ADD_EXECUTABLE(${name})
    BUZ_TIER(${name} ${GAME_TIER})

    FILE(GLOB_RECURSE game_sources CONFIGURE_DEPENDS
         "${CMAKE_CURRENT_LIST_DIR}/private/*.cpp"
         "${CMAKE_CURRENT_LIST_DIR}/private/*.h"
         "${CMAKE_CURRENT_LIST_DIR}/private/*.hpp")
    TARGET_SOURCES(${name} PRIVATE ${game_sources})
    GROUPSOURCES(${CMAKE_CURRENT_LIST_DIR}/private private)

    TARGET_LINK_LIBRARIES(${name} PRIVATE ${GAME_LINKS})
    BUZ_COMPILE_OPTIONS(${name})

    BUZ_COMPILE_SHADERS(${name})
    ADD_DEPENDENCIES(${name} copy-content check_stray_sources)

    IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/content)
        ADD_CUSTOM_COMMAND(TARGET ${name} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_directory "${CMAKE_CURRENT_LIST_DIR}/content" "$<TARGET_FILE_DIR:${name}>/content"
            COMMENT "Copying the ${name} content next to the executable"
            VERBATIM)
    ENDIF()

    # In a browser the game is a page: the executable becomes WebAssembly
    # plus the html that starts it, and the content is packed into one .data
    # file the page downloads. The browser has no folder next to the
    # executable, so the content goes where paths::get_content_directory
    # looks, /content.
    IF(EMSCRIPTEN)
        SET(web_content
            "${CMAKE_SOURCE_DIR}/content"
            "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content/shaders@/content/shaders")
        IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/content)
            LIST(APPEND web_content "${CMAKE_CURRENT_LIST_DIR}/content")
        ENDIF()
        SET(web_link_options
            -sMIN_WEBGL_VERSION=2 -sMAX_WEBGL_VERSION=2
            -sGL_ENABLE_GET_PROC_ADDRESS=1
            -sALLOW_MEMORY_GROWTH=1
            -sSTACK_SIZE=1MB
            -sEXPORTED_RUNTIME_METHODS=callMain
            "SHELL:--shell-file \"${CMAKE_SOURCE_DIR}/cmake/web_shell.html\"")
        FOREACH(folder ${web_content})
            IF(NOT folder MATCHES "@")
                SET(folder "${folder}@/content")
            ENDIF()
            LIST(APPEND web_link_options "SHELL:--preload-file \"${folder}\"")
        ENDFOREACH()
        TARGET_LINK_OPTIONS(${name} PRIVATE ${web_link_options})
        SET_TARGET_PROPERTIES(${name} PROPERTIES SUFFIX ".html")

        # The content is linked in, so a changed file means linking again
        FILE(GLOB_RECURSE web_content_files CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/content/*" "${CMAKE_CURRENT_LIST_DIR}/content/*")
        SET_TARGET_PROPERTIES(${name} PROPERTIES LINK_DEPENDS "${web_content_files}")
    ENDIF()

    IF(NOT EMSCRIPTEN)
        BUZ_PACKAGE_TARGETS(${name})
    ENDIF()

    IF(GAME_TIER STREQUAL "staff")
        SET_TARGET_PROPERTIES(${name} PROPERTIES FOLDER "games/staff")
    ELSE()
        SET_TARGET_PROPERTIES(${name} PROPERTIES FOLDER "games")
    ENDIF()
ENDFUNCTION()

#--------------------------------------------------------------
# The package targets of one game. They depend on the game, so the IDE
# builds it in the selected configuration first and the package script only
# collects the result. A web package builds in its own tree (build-web/)
# with the shader_compiler of this one.
FUNCTION(BUZ_PACKAGE_TARGETS name)
    SET(package_build "$<$<CONFIG:Release>:shipping>$<$<CONFIG:Development>:development>$<$<CONFIG:Debug>:debug>")
    SET(package_command ${CMAKE_COMMAND} -DGAME=${name} -DBUILD=${package_build} "-DBUILD_DIRECTORY=${CMAKE_BINARY_DIR}")
    SET(package_script "${CMAKE_SOURCE_DIR}/cmake/package.cmake")

    ADD_CUSTOM_TARGET(package_${name}
        COMMAND ${package_command} -DALREADY_BUILT=ON -P "${package_script}"
        COMMENT "Packaging ${name}"
        USES_TERMINAL VERBATIM)
    ADD_DEPENDENCIES(package_${name} ${name})
    SET_TARGET_PROPERTIES(package_${name} PROPERTIES FOLDER package)

    IF(BUZ_RHI STREQUAL "opengl")
        ADD_CUSTOM_TARGET(package_${name}_web
            COMMAND ${package_command} -DPLATFORM=web "-DSHADER_COMPILER=$<TARGET_FILE:shader_compiler>" -P "${package_script}"
            COMMENT "Packaging ${name} for the web"
            USES_TERMINAL VERBATIM)
        ADD_DEPENDENCIES(package_${name}_web shader_compiler)
        SET_TARGET_PROPERTIES(package_${name}_web PROPERTIES FOLDER package)
    ENDIF()
ENDFUNCTION()

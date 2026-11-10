# -------------------------------
# Guard against a classic visual studio trap.
#
# The build/ folder is generated: it can be deleted and rebuilt from scratch
# at any time, so nothing you write by hand belongs in it. But visual studio's
# "Add > New Item..." dialog defaults to the project directory, which lives
# INSIDE build/ - a source file created there compiles once, then silently
# drops out of the project the next time CMake regenerates it.
#
# This script looks for hand-made source files inside build/ and prints a
# warning for each one, formatted so visual studio shows it in the Error List.
# It runs once at generate time and again on every build (see the
# check_stray_sources target below, which every game depends on).
# -------------------------------
FUNCTION(BUZ_CHECK_STRAY_SOURCES build_dir)
    FILE(GLOB_RECURSE stray_sources
         "${build_dir}/source/*.cpp"
         "${build_dir}/source/*.h"
         "${build_dir}/source/*.hpp")

    FOREACH(stray_source ${stray_sources})
        # CMake generates helper sources of its own inside CMakeFiles/ folders;
        # those are not hand-made files, so skip them.
        IF(stray_source MATCHES "/CMakeFiles/")
            CONTINUE()
        ENDIF()

        # Libraries configure headers of their own into the build folder as
        # well (SDL writes its build config there); those are not yours either.
        IF(stray_source MATCHES "/source/third_party/")
            CONTINUE()
        ENDIF()

        # "<path>(<line>): warning : <text>" is the format visual studio
        # recognizes, so the message lands in the Error List and double
        # clicking it opens the stray file.
        MESSAGE("${stray_source}(1): warning : This file was created inside the disposable build/ folder - fine for a quick test, but it is NOT part of your game and will be lost when the project regenerates. Move it to source/games/<your_game>/private/ - run docs.bat and read \"Adding files to your game\" in the getting started guide.")
    ENDFOREACH()
ENDFUNCTION()

IF(CMAKE_SCRIPT_MODE_FILE)
    # Run as a standalone script (cmake -P): this is what the
    # check_stray_sources target does on every build.
    BUZ_CHECK_STRAY_SOURCES(${BUILD_DIR})
ELSE()
    # Included from the root CMakeLists.txt: check once right away (so
    # generate.bat also warns) and set up the target that re-checks on every
    # build. A custom target without outputs is never "up to date", which is
    # exactly what we want here.
    BUZ_CHECK_STRAY_SOURCES(${CMAKE_BINARY_DIR})

    ADD_CUSTOM_TARGET(check_stray_sources
        COMMAND ${CMAKE_COMMAND} -DBUILD_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/check_stray_sources.cmake
        COMMENT "Checking for source files accidentally created in the build folder"
        VERBATIM)
    SET_PROPERTY(TARGET check_stray_sources PROPERTY FOLDER "events")
ENDIF()

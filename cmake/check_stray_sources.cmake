# -------------------------------
# Warns about source files created inside build/ by mistake.
#
# The build/ folder is generated: it can be deleted and rebuilt from scratch
# at any time, so nothing you write by hand belongs in it. Visual Studio's
# "Add > New Item..." dialog, however, defaults to the project directory,
# which lies inside build/. A source file created there compiles once and
# then silently drops out of the project the next time CMake regenerates it.
#
# This script looks for hand-made source files inside build/ and prints a
# warning for each one, formatted so Visual Studio shows it in the Error List.
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
        # well (SDL writes its build config there); those are not hand-made
        # either.
        IF(stray_source MATCHES "/source/third_party/")
            CONTINUE()
        ENDIF()

        # "<path>(<line>): warning : <text>" is the format Visual Studio
        # recognizes, so the message lands in the Error List and double
        # clicking it opens the stray file.
        MESSAGE("${stray_source}(1): warning : This file was created inside the disposable build/ folder. That is fine for a quick test, but the file is not part of your game and will be lost when the project regenerates. Move it to source/games/<your_game>/private/; for the steps, run open_docs.bat (open_docs.sh on Linux) and read \"Adding files to your game\" in the getting started guide.")
    ENDFOREACH()
ENDFUNCTION()

IF(CMAKE_SCRIPT_MODE_FILE)
    # Run as a standalone script (cmake -P): this is what the
    # check_stray_sources target does on every build.
    BUZ_CHECK_STRAY_SOURCES(${BUILD_DIR})
ELSE()
    # Included from the root CMakeLists.txt: check once right away (so
    # generate.bat also warns) and set up the target that re-checks on every
    # build. A custom target without outputs is never "up to date", so the
    # check runs on every build.
    BUZ_CHECK_STRAY_SOURCES(${CMAKE_BINARY_DIR})

    ADD_CUSTOM_TARGET(check_stray_sources
        COMMAND ${CMAKE_COMMAND} -DBUILD_DIR=${CMAKE_BINARY_DIR} -P ${CMAKE_SOURCE_DIR}/cmake/check_stray_sources.cmake
        COMMENT "Checking for source files accidentally created in the build folder"
        VERBATIM)
    SET_PROPERTY(TARGET check_stray_sources PROPERTY FOLDER "events")
ENDIF()

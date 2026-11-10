# -------------------------------
# Packages one game for sharing: an optimized build plus everything it needs
# to run somewhere else, in package/.
#
#     cmake -DGAME=sample_2d [-DPLATFORM=web] [-DBUILD=development] -P cmake/package.cmake
#
#   desktop (the default)  <game>-windows.zip or <game>-linux.tar.gz (which
#                          keeps the executable bit): the executable and its
#                          content folder. SDL is linked statically, so that
#                          is the whole game.
#   web                    <game>-web.zip: a page with the game compiled to
#                          WebAssembly, for itch.io or a portfolio site.
#
#   shipping (the default) the Release build: -O3 and link-time
#                          optimization, no logging.
#   development            the Development build: -O1 with logging, to test
#                          a package; its name ends in -development.
#
# package.bat and package.sh call this script, so both platforms share one
# implementation. A desktop package is built from build/, so it draws with
# whichever api generate configured.
#
# The IDE calls it too, through each game's package targets (see
# cmake/buz_game.cmake). The IDE has built the game already, so it passes
#   BUILD_DIRECTORY   its build folder, instead of build/
#   ALREADY_BUILT     ON: package the desktop executable as it is
#   SHADER_COMPILER   the shader_compiler a web package uses
# -------------------------------
CMAKE_MINIMUM_REQUIRED(VERSION 3.21)

IF(NOT GAME)
    MESSAGE(FATAL_ERROR "Which game? Usage: cmake -DGAME=<game> [-DPLATFORM=web] [-DBUILD=development] -P cmake/package.cmake")
ENDIF()

# The build configuration (see the root CMakeLists.txt)
IF(NOT BUILD OR BUILD STREQUAL "shipping")
    SET(BUILD shipping)
    SET(config Release)
    SET(name_suffix "")
ELSEIF(BUILD STREQUAL "development")
    SET(config Development)
    SET(name_suffix "-development")
ELSEIF(BUILD STREQUAL "debug")
    MESSAGE(FATAL_ERROR "A package is a Release (shipping) or Development build: switch the build configuration from Debug.")
ELSE()
    MESSAGE(FATAL_ERROR "BUILD is \"${BUILD}\"; use shipping or development.")
ENDIF()

GET_FILENAME_COMPONENT(root "${CMAKE_CURRENT_LIST_DIR}/.." ABSOLUTE)
IF(BUILD_DIRECTORY)
    SET(build_directory "${BUILD_DIRECTORY}")
ELSE()
    SET(build_directory "${root}/build")
ENDIF()
SET(package_directory "${root}/package")

IF(NOT EXISTS "${build_directory}/CMakeCache.txt")
    MESSAGE(FATAL_ERROR "No build/ folder yet; run generate first.")
ENDIF()

IF(CMAKE_HOST_WIN32)
    SET(host_os windows)
    SET(executable_suffix .exe)
ELSE()
    SET(host_os linux)
    SET(executable_suffix "")
ENDIF()

IF(NOT PLATFORM)
    SET(PLATFORM ${host_os})
ENDIF()
IF(NOT PLATFORM STREQUAL host_os AND NOT PLATFORM STREQUAL "web")
    MESSAGE(FATAL_ERROR "Cannot package for ${PLATFORM} on ${host_os}; a desktop package is made on its own OS.")
ENDIF()

#--------------------------------------------------------------
# Runs a command and stops packaging with the given message when it fails.
FUNCTION(RUN_OR_FAIL message)
    EXECUTE_PROCESS(COMMAND ${ARGN} RESULT_VARIABLE result)
    IF(NOT result EQUAL 0)
        MESSAGE(FATAL_ERROR "${message}")
    ENDIF()
ENDFUNCTION()

#--------------------------------------------------------------
# Downloads a file once; a cached copy is used from then on.
FUNCTION(DOWNLOAD_ONCE url file)
    IF(NOT EXISTS "${file}")
        MESSAGE(STATUS "Downloading ${url}")
        FILE(DOWNLOAD "${url}" "${file}.part" STATUS status SHOW_PROGRESS)
        LIST(GET status 0 code)
        IF(NOT code EQUAL 0)
            FILE(REMOVE "${file}.part")
            MESSAGE(FATAL_ERROR "Could not download ${url}: ${status}")
        ENDIF()
        FILE(RENAME "${file}.part" "${file}")
    ENDIF()
ENDFUNCTION()

IF(PLATFORM STREQUAL "web")
    # A browser runs OpenGL ES (WebGL 2), so the shaders are translated by
    # the same shader_compiler the OpenGL build uses: the web package reuses
    # it rather than building a second one for the host.
    FILE(STRINGS "${build_directory}/CMakeCache.txt" rhi REGEX "^BUZ_RHI:")
    IF(NOT rhi MATCHES "=opengl$")
        MESSAGE(FATAL_ERROR "A web package needs the OpenGL build: run generate without a preset first.")
    ENDIF()
    IF(SHADER_COMPILER)
        SET(shader_compiler "${SHADER_COMPILER}")
    ELSE()
        RUN_OR_FAIL("Building shader_compiler failed."
            ${CMAKE_COMMAND} --build "${build_directory}" --target shader_compiler --config Release)
        SET(shader_compiler "${build_directory}/Release/shader_compiler${executable_suffix}")
    ENDIF()

    # The Emscripten SDK compiles C++ to WebAssembly. It is about 1 GB, so
    # it is not part of the project: it is downloaded the first time and
    # kept in a per-user cache that every project shares. The versions are
    # pinned, so every machine builds the same page.
    SET(emsdk_version 6.0.11)
    SET(python_version 3.13.3)
    SET(ninja_version 1.13.2)
    IF(CMAKE_HOST_WIN32)
        FILE(TO_CMAKE_PATH "$ENV{LOCALAPPDATA}/buz" cache_directory)
    ELSEIF(DEFINED ENV{XDG_CACHE_HOME})
        SET(cache_directory "$ENV{XDG_CACHE_HOME}/buz")
    ELSE()
        SET(cache_directory "$ENV{HOME}/.cache/buz")
    ENDIF()
    SET(emsdk_directory "${cache_directory}/emsdk-${emsdk_version}")

    # The SDK's installer and compiler driver are Python scripts. Linux has
    # Python; on Windows the official embeddable one is fetched, and Ninja
    # with it (Visual Studio projects cannot target the browser).
    IF(CMAKE_HOST_WIN32)
        SET(python_directory "${cache_directory}/python-${python_version}")
        SET(python "${python_directory}/python.exe")
        IF(NOT EXISTS "${python}")
            DOWNLOAD_ONCE("https://www.python.org/ftp/python/${python_version}/python-${python_version}-embed-amd64.zip" "${python_directory}.zip")
            FILE(ARCHIVE_EXTRACT INPUT "${python_directory}.zip" DESTINATION "${python_directory}")
        ENDIF()

        SET(ninja_directory "${cache_directory}/ninja-${ninja_version}")
        SET(ninja "${ninja_directory}/ninja.exe")
        IF(NOT EXISTS "${ninja}")
            DOWNLOAD_ONCE("https://github.com/ninja-build/ninja/releases/download/v${ninja_version}/ninja-win.zip" "${ninja_directory}.zip")
            FILE(ARCHIVE_EXTRACT INPUT "${ninja_directory}.zip" DESTINATION "${ninja_directory}")
        ENDIF()
    ELSE()
        FIND_PROGRAM(python python3 REQUIRED)
        FIND_PROGRAM(ninja ninja REQUIRED)
    ENDIF()

    # Written last, so an interrupted download or install starts over
    SET(emsdk_installed "${emsdk_directory}/buz_installed")
    IF(NOT EXISTS "${emsdk_installed}")
        MESSAGE(STATUS "Installing the Emscripten SDK ${emsdk_version} into ${emsdk_directory}; this happens once and takes a few minutes")
        DOWNLOAD_ONCE("https://github.com/emscripten-core/emsdk/archive/refs/tags/${emsdk_version}.zip" "${emsdk_directory}.zip")
        FILE(REMOVE_RECURSE "${emsdk_directory}")
        FILE(ARCHIVE_EXTRACT INPUT "${emsdk_directory}.zip" DESTINATION "${cache_directory}")
        RUN_OR_FAIL("Installing the Emscripten SDK failed."
            "${python}" "${emsdk_directory}/emsdk.py" install ${emsdk_version})
        RUN_OR_FAIL("Activating the Emscripten SDK failed."
            "${python}" "${emsdk_directory}/emsdk.py" activate ${emsdk_version})
        FILE(TOUCH "${emsdk_installed}")
    ENDIF()

    # emcc finds Python through this variable; every command below inherits
    # it. On Windows the SDK installs a full Python of its own: the
    # embeddable one only installs the SDK, since it keeps a script's own
    # folder off the module path, which emcc needs.
    IF(CMAKE_HOST_WIN32)
        FILE(GLOB emsdk_python "${emsdk_directory}/python/*/python.exe")
        LIST(GET emsdk_python 0 python)
    ENDIF()
    SET(ENV{EMSDK_PYTHON} "${python}")

    # One tree per build: switching one tree would rebuild everything
    SET(web_build_directory "${root}/build-web/${BUILD}")
    RUN_OR_FAIL("Configuring the web build failed."
        ${CMAKE_COMMAND} -S "${root}" -B "${web_build_directory}" -G Ninja
        "-DCMAKE_MAKE_PROGRAM=${ninja}"
        "-DCMAKE_TOOLCHAIN_FILE=${emsdk_directory}/upstream/emscripten/cmake/Modules/Platform/Emscripten.cmake"
        -DCMAKE_BUILD_TYPE=${config}
        "-DBUZ_SHADER_COMPILER=${shader_compiler}")
    RUN_OR_FAIL("Building ${GAME} failed; is \"${GAME}\" a folder in source/games?"
        ${CMAKE_COMMAND} --build "${web_build_directory}" --target ${GAME})

    # A web host opens index.html; the page loads the rest by name
    SET(package_name "${GAME}-web${name_suffix}")
    SET(staging_directory "${package_directory}/${package_name}")
    FILE(REMOVE_RECURSE "${staging_directory}")
    FILE(MAKE_DIRECTORY "${staging_directory}")
    SET(web_output "${web_build_directory}/${config}")
    FILE(COPY_FILE "${web_output}/${GAME}.html" "${staging_directory}/index.html")
    FILE(COPY "${web_output}/${GAME}.js" "${web_output}/${GAME}.wasm" "${web_output}/${GAME}.data"
         DESTINATION "${staging_directory}")
    # itch.io wants index.html at the top of the zip, not in a folder
    SET(archive_flags cf)
    SET(archive_format zip)
    SET(archive_extension zip)
    SET(archive_working_directory "${staging_directory}")
    SET(archive_inputs index.html ${GAME}.js ${GAME}.wasm ${GAME}.data)
ELSE()
    # Both generators the presets use (Visual Studio, Ninja Multi-Config)
    # keep every configuration side by side, so this never touches the
    # Debug build.
    IF(NOT ALREADY_BUILT)
        MESSAGE(STATUS "Building the ${config} version of ${GAME}")
        RUN_OR_FAIL("Building ${GAME} failed; is \"${GAME}\" a folder in source/games?"
            ${CMAKE_COMMAND} --build "${build_directory}" --target ${GAME} --config ${config})
    ENDIF()

    # The game finds its content next to the executable
    # (paths::get_content_directory). build/<config>/content holds the shared
    # content, the compiled shaders and the content of every game in this
    # tree; all of it is small, so all of it goes.
    SET(package_name "${GAME}-${host_os}${name_suffix}")
    SET(staging_directory "${package_directory}/${package_name}")
    FILE(REMOVE_RECURSE "${staging_directory}")
    FILE(MAKE_DIRECTORY "${staging_directory}")
    FILE(COPY "${build_directory}/${config}/${GAME}${executable_suffix}" "${build_directory}/${config}/content"
         DESTINATION "${staging_directory}")
    IF(CMAKE_HOST_WIN32)
        SET(archive_flags cf)
        SET(archive_format zip)
        SET(archive_extension zip)
    ELSE()
        SET(archive_flags cfz)
        SET(archive_format gnutar)
        SET(archive_extension tar.gz)
    ENDIF()
    SET(archive_working_directory "${package_directory}")
    SET(archive_inputs "${package_name}")
ENDIF()

# The folder stays next to the archive, so the package can be tried out
# before it is shared
SET(archive "${package_directory}/${package_name}.${archive_extension}")
FILE(REMOVE "${archive}")
EXECUTE_PROCESS(
    COMMAND ${CMAKE_COMMAND} -E tar ${archive_flags} "${archive}" --format=${archive_format} ${archive_inputs}
    WORKING_DIRECTORY "${archive_working_directory}"
    RESULT_VARIABLE archive_result)
IF(NOT archive_result EQUAL 0)
    MESSAGE(FATAL_ERROR "Could not create ${archive}.")
ENDIF()

MESSAGE(STATUS "Packaged ${GAME}: ${archive}")

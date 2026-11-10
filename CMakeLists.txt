cmake_minimum_required(VERSION 3.25)

project(buz_engine LANGUAGES C CXX)

# Windows (DirectX 12, Vulkan and OpenGL) and Linux (Vulkan and OpenGL). The
# browser (Emscripten) is a package target only: cmake/package.cmake
# configures it, with OpenGL ES (WebGL 2).
IF(NOT WIN32 AND NOT CMAKE_SYSTEM_NAME STREQUAL "Linux" AND NOT EMSCRIPTEN)
    MESSAGE(FATAL_ERROR "BuzEngine builds on Windows and Linux.")
ENDIF()
IF(EMSCRIPTEN)
    SET(BUZ_RHI opengl CACHE STRING "" FORCE)
    IF(NOT BUZ_SHADER_COMPILER)
        MESSAGE(FATAL_ERROR "A web build compiles its shaders with the shader_compiler of a desktop OpenGL build: set BUZ_SHADER_COMPILER, or let package -web do it.")
    ENDIF()
ENDIF()

# The graphics api the engine draws with. A build compiles exactly one rhi
# backend (source/engine/rhi/private/<api>): OpenGL by default, the api
# students start with, or DirectX 12 (Windows only) or Vulkan. The staff
# presets build the other backends in second trees: vs2026-d3d12
# (build-d3d12/), vs2026-vulkan and linux-vulkan (build-vulkan/).
SET(BUZ_RHI opengl CACHE STRING "Graphics api the engine draws with: d3d12 (Windows only), vulkan or opengl")
SET_PROPERTY(CACHE BUZ_RHI PROPERTY STRINGS d3d12 vulkan opengl)
IF(NOT EXISTS ${CMAKE_CURRENT_LIST_DIR}/source/engine/rhi/private/${BUZ_RHI})
    MESSAGE(FATAL_ERROR "BUZ_RHI is \"${BUZ_RHI}\", but there is no such backend in source/engine/rhi/private.")
ENDIF()
IF(BUZ_RHI STREQUAL "d3d12" AND NOT WIN32)
    MESSAGE(FATAL_ERROR "DirectX 12 only builds on Windows; use -DBUZ_RHI=vulkan or -DBUZ_RHI=opengl.")
ENDIF()
MESSAGE(STATUS "Graphics api - ${BUZ_RHI}")

# Three builds, as in Unreal: Debug to step through, Development (-O1,
# logging on) to test a build at speed, and Release, the shipping build:
# the highest optimization the compiler has (-O3, MSVC /O2 with the most
# inlining), link-time optimization across every library, and no logging.
# The packager makes Development and Release packages. MSVC has no -O1
# level; its /O1 (small code) is the nearest step below /O2.
SET(CMAKE_CONFIGURATION_TYPES Debug Development Release)
IF(MSVC)
    SET(development_optimization "/O1 /Ob1")
    FOREACH(language C CXX)
        SET(CMAKE_${language}_FLAGS_RELEASE "/O2 /Ob3 /DNDEBUG")
    ENDFOREACH()
ELSE()
    SET(development_optimization "-O1")
    FOREACH(language C CXX)
        SET(CMAKE_${language}_FLAGS_RELEASE "-O3 -DNDEBUG")
    ENDFOREACH()
ENDIF()
FOREACH(language C CXX)
    SET(CMAKE_${language}_FLAGS_DEVELOPMENT "${development_optimization} -DNDEBUG -DBUZ_DEVELOPMENT")
ENDFOREACH()
FOREACH(kind EXE SHARED MODULE)
    SET(CMAKE_${kind}_LINKER_FLAGS_DEVELOPMENT "${CMAKE_${kind}_LINKER_FLAGS_RELEASE}")
ENDFOREACH()
IF(EMSCRIPTEN)
    # emcc optimizes the WebAssembly again when it links
    SET(CMAKE_EXE_LINKER_FLAGS_RELEASE "-O3")
    SET(CMAKE_EXE_LINKER_FLAGS_DEVELOPMENT "-O1")
ENDIF()

# Link-time optimization (MSVC /GL and /LTCG, -flto elsewhere) lets the
# compiler inline and drop code across libraries, the engine and SDL
# included. Release only: it makes linking slower.
INCLUDE(CheckIPOSupported)
CHECK_IPO_SUPPORTED(RESULT link_time_optimization OUTPUT link_time_optimization_error LANGUAGES C CXX)
IF(link_time_optimization)
    SET(CMAKE_INTERPROCEDURAL_OPTIMIZATION_RELEASE ON)
ELSE()
    MESSAGE(WARNING "No link-time optimization for Release: ${link_time_optimization_error}")
ENDIF()

# Use C++ 20 as a standard, the same standard current game consoles compile with
SET(CMAKE_CXX_STANDARD 20)
SET(CMAKE_CXX_STANDARD_REQUIRED ON)

# Enable folder structure in the IDE
SET_PROPERTY(GLOBAL PROPERTY USE_FOLDERS ON)

# Set output directories for each configuration (Debug, Release, etc.)
SET(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
SET(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)
SET(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/$<CONFIG>)

# Debug postfix
SET(CMAKE_DEBUG_POSTFIX _d)

# Some easy access to folders within this repository
SET(SOURCE_ENGINE_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/engine)
SET(SOURCE_TOOLS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/tools)
SET(SOURCE_GAMES_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/games)
SET(SOURCE_TESTS_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/tests)
SET(SOURCE_THIRDPARTY_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/source/third_party)
SET(SOURCE_SHADER_DIRECTORY ${CMAKE_CURRENT_LIST_DIR}/shaders)

# The heavier build logic lives in its own script per topic inside the /cmake
# folder; every script is commented if you are curious how it works.
LIST(APPEND CMAKE_MODULE_PATH ${CMAKE_CURRENT_LIST_DIR}/cmake)
INCLUDE(group_sources)       # GROUPSOURCES: mirror a folder tree as IDE filters
INCLUDE(compile_shaders)     # BUZ_COMPILE_SHADERS: compile the /shaders hlsl files
INCLUDE(copy_content)        # copy-content: copy /content next to the executable
INCLUDE(check_stray_sources) # warn about source files accidentally created in build/
INCLUDE(buz_tiers)          # BUZ_TIER: which student template a target ships in
INCLUDE(buz_compile_options) # BUZ_COMPILE_OPTIONS: the settings of buz's own targets
INCLUDE(buz_module)         # BUZ_MODULE: one engine module, one static library
INCLUDE(buz_game)           # BUZ_GAME: one game, one executable
IF(EXISTS ${CMAKE_CURRENT_LIST_DIR}/docs)
    INCLUDE(docs_target)     # optional "docs" target when doxygen is installed
ENDIF()

# SYSTEM: vendored headers are system headers, so buz's warnings-as-errors
# (cmake/buz_compile_options.cmake) never fire on code we do not own
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY} SYSTEM)
ADD_SUBDIRECTORY(${SOURCE_ENGINE_DIRECTORY})
# The tools run on the machine that builds, so a web build has none
IF(NOT EMSCRIPTEN)
    ADD_SUBDIRECTORY(${SOURCE_TOOLS_DIRECTORY})
ENDIF()
ADD_SUBDIRECTORY(${SOURCE_GAMES_DIRECTORY})

# The unit tests (staff only, so templates have no tests folder): run them
# with ctest --test-dir build -C Debug
IF(EXISTS ${SOURCE_TESTS_DIRECTORY} AND NOT EMSCRIPTEN)
    ENABLE_TESTING()
    ADD_SUBDIRECTORY(${SOURCE_TESTS_DIRECTORY})
ENDIF()

# Refuse to configure when a target has no tier, or links a target of a
# higher tier (see cmake/buz_tiers.cmake).
BUZ_CHECK_TIERS()

# Make the simple starter game the startup project when opening the solution in
# Visual Studio; right-click a different game and "Set as Startup Project" to
# debug that one instead.
SET_PROPERTY(DIRECTORY ${CMAKE_CURRENT_LIST_DIR} PROPERTY VS_STARTUP_PROJECT sample_2d)

MESSAGE(STATUS "Adding third party libraries")

ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/stb)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/rnd)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/args)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/nlohmann)
IF(NOT EMSCRIPTEN)
    ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/dxc)
ENDIF()

# glm is vendored as the full upstream repository, which ships its own build
# scripts (tests, install rules, ...) that we do not need. Instead of adding
# that subdirectory we define our own thin header-only target here, following
# the same pattern as nlohmann.
ADD_LIBRARY(glm INTERFACE)
TARGET_INCLUDE_DIRECTORIES(glm INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/glm)

# -------------------------------
# SDL3: window, input and events on every platform. Vendored as source and
# built as a static library, so there is no SDL3.dll to copy around. The
# options below switch off everything the engine does not need (tests,
# examples, install rules) before SDL's own CMakeLists.txt reads them.
# -------------------------------
SET(SDL_SHARED OFF CACHE BOOL "" FORCE)
SET(SDL_STATIC ON CACHE BOOL "" FORCE)
SET(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
SET(SDL_TESTS OFF CACHE BOOL "" FORCE)
SET(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)
SET(SDL_INSTALL OFF CACHE BOOL "" FORCE)
SET(SDL_UNINSTALL OFF CACHE BOOL "" FORCE)

# SDL's subsystems: the engine uses the window and its events (Video), and
# Joystick and HIDAPI stay for gamepads (HIDAPI drives PlayStation, Switch
# and most other pads; rumble goes through the gamepad api, not Haptic).
# The rest is not compiled at all: miniaudio plays the sound, the rhi does
# the drawing, so a first build has less to compile.
SET(SDL_AUDIO OFF CACHE BOOL "" FORCE)
SET(SDL_GPU OFF CACHE BOOL "" FORCE)
SET(SDL_RENDER OFF CACHE BOOL "" FORCE)
SET(SDL_CAMERA OFF CACHE BOOL "" FORCE)
SET(SDL_HAPTIC OFF CACHE BOOL "" FORCE)
SET(SDL_POWER OFF CACHE BOOL "" FORCE)
SET(SDL_SENSOR OFF CACHE BOOL "" FORCE)
SET(SDL_DIALOG OFF CACHE BOOL "" FORCE)
SET(SDL_TRAY OFF CACHE BOOL "" FORCE)
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/sdl3 EXCLUDE_FROM_ALL)
SET_TARGET_PROPERTIES(SDL3-static PROPERTIES FOLDER "third-party")
IF(TARGET SDL_uclibc)
    SET_TARGET_PROPERTIES(SDL_uclibc PROPERTIES FOLDER "third-party")
ENDIF()

# xyesno: named yes/no types instead of literal booleans in the engine api
ADD_SUBDIRECTORY(${SOURCE_THIRDPARTY_DIRECTORY}/xyesno)
SET_TARGET_PROPERTIES(yesno PROPERTIES FOLDER "third-party")

# -------------------------------
# Vulkan: the official headers, volk (loads the Vulkan functions from the
# driver at runtime, so no Vulkan SDK or import library is needed to build)
# and the Vulkan Memory Allocator (VMA, gpu memory management). Only the
# Vulkan backend in source/engine/rhi uses them, so they are only part of a
# Vulkan build.
# -------------------------------
IF(BUZ_RHI STREQUAL "vulkan")
    ADD_LIBRARY(vulkan_headers INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(vulkan_headers INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/vulkan/include)
    # volk declares every Vulkan function itself; the prototypes from vulkan.h
    # would clash with those.
    TARGET_COMPILE_DEFINITIONS(vulkan_headers INTERFACE VK_NO_PROTOTYPES)

    ADD_LIBRARY(volk STATIC ${SOURCE_THIRDPARTY_DIRECTORY}/volk/volk.c ${SOURCE_THIRDPARTY_DIRECTORY}/volk/volk.h)
    TARGET_INCLUDE_DIRECTORIES(volk PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/volk)
    TARGET_LINK_LIBRARIES(volk PUBLIC vulkan_headers ${CMAKE_DL_LIBS})
    SET_TARGET_PROPERTIES(volk PROPERTIES FOLDER "third-party")

    # vma is a single header; its implementation is compiled once, inside the
    # Vulkan backend.
    ADD_LIBRARY(vma INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(vma INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/vma/include)
    TARGET_LINK_LIBRARIES(vma INTERFACE vulkan_headers)
ENDIF()

# -------------------------------
# OpenGL: the official Khronos headers, which declare the OpenGL types,
# constants and function pointer types. There is no OpenGL library to link:
# the backend in source/engine/rhi loads every function from the driver at
# runtime. SPIRV-Cross (the core and its GLSL writer only) lets
# shader_compiler turn the SPIR-V dxc makes into GLSL; it is part of the
# tool, never of a game.
# -------------------------------
IF(BUZ_RHI STREQUAL "opengl")
    ADD_LIBRARY(opengl_headers INTERFACE)
    TARGET_INCLUDE_DIRECTORIES(opengl_headers INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/opengl/include)
ENDIF()

# The tools never run in a browser
IF(BUZ_RHI STREQUAL "opengl" AND NOT EMSCRIPTEN)
    FILE(GLOB spirv_cross_sources ${SOURCE_THIRDPARTY_DIRECTORY}/spirv_cross/*.cpp ${SOURCE_THIRDPARTY_DIRECTORY}/spirv_cross/*.hpp ${SOURCE_THIRDPARTY_DIRECTORY}/spirv_cross/*.h)
    ADD_LIBRARY(spirv_cross STATIC ${spirv_cross_sources})
    TARGET_INCLUDE_DIRECTORIES(spirv_cross PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/spirv_cross)
    SET_TARGET_PROPERTIES(spirv_cross PROPERTIES FOLDER "third-party")
ENDIF()

# -------------------------------
# miniaudio (0.11.25): sound effects and music. A single header; its
# implementation is compiled once, in the audio module.
# -------------------------------
ADD_LIBRARY(miniaudio INTERFACE)
TARGET_INCLUDE_DIRECTORIES(miniaudio INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/miniaudio)

# -------------------------------
# cgltf: reads glTF 2.0 models (.gltf and .glb). A single header; its
# implementation is compiled once, in the scene module.
# -------------------------------
ADD_LIBRARY(cgltf INTERFACE)
TARGET_INCLUDE_DIRECTORIES(cgltf INTERFACE ${SOURCE_THIRDPARTY_DIRECTORY}/cgltf)

# -------------------------------
# Catch2 (3.16.0, the two-file amalgamated build): the unit tests in
# source/tests. Staff only: templates have neither the tests nor Catch2.
# -------------------------------
IF(EXISTS ${SOURCE_THIRDPARTY_DIRECTORY}/catch2)
    ADD_LIBRARY(catch2 STATIC ${SOURCE_THIRDPARTY_DIRECTORY}/catch2/catch_amalgamated.cpp ${SOURCE_THIRDPARTY_DIRECTORY}/catch2/catch_amalgamated.hpp)
    TARGET_INCLUDE_DIRECTORIES(catch2 PUBLIC ${SOURCE_THIRDPARTY_DIRECTORY}/catch2)
    SET_TARGET_PROPERTIES(catch2 PROPERTIES FOLDER "third-party")
ENDIF()

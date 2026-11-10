MESSAGE(STATUS "Adding tool - shader_compiler")
ADD_EXECUTABLE(shader_compiler)
BUZ_TIER(shader_compiler tmpl_2d)

# Sources
TARGET_SOURCES(shader_compiler PRIVATE
    ${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private/shader_compiler_main.cpp)

# Create the project filters
GROUPSOURCES(${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private private)

# The dxc compiler api (dxcapi.h + the dxcompiler library) is vendored in third_party/dxc
TARGET_LINK_LIBRARIES(shader_compiler PRIVATE dxc)
BUZ_COMPILE_OPTIONS(shader_compiler)

# An OpenGL build also translates the SPIR-V dxc makes into GLSL, with
# SPIRV-Cross. Only that build compiles it.
IF(BUZ_RHI STREQUAL "opengl")
    TARGET_SOURCES(shader_compiler PRIVATE
        ${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private/glsl.cpp
        ${SOURCE_TOOLS_DIRECTORY}/shader_compiler/private/glsl.h)
    TARGET_LINK_LIBRARIES(shader_compiler PRIVATE spirv_cross)
    TARGET_COMPILE_DEFINITIONS(shader_compiler PRIVATE BUZ_SHADER_GLSL)
ENDIF()

# Target properties
SET_TARGET_PROPERTIES(shader_compiler PROPERTIES FOLDER "tools")

# -------------------------------
# The tool needs the dxc libraries (dxcompiler + dxil) at runtime, so copy
# them from the vendored release next to the tool executable. On Linux the
# executable is told to look for them there ($ORIGIN is its own folder).
# -------------------------------
ADD_CUSTOM_COMMAND(TARGET shader_compiler POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different ${DXC_RUNTIME_LIBRARIES} "$<TARGET_FILE_DIR:shader_compiler>"
    COMMENT "Copying the dxc libraries next to shader_compiler"
    VERBATIM)

IF(NOT WIN32)
    SET_TARGET_PROPERTIES(shader_compiler PROPERTIES BUILD_RPATH "$ORIGIN")
ENDIF()

# -------------------------------
# Function that hooks shader compilation into the build.
#
# Every *.hlsl file inside the /shaders folder, and in the shaders/ folder of
# an engine module that brings its own (the staff ps1_look), is compiled into
# the runtime output folder (content/shaders) using our own shader_compiler
# tool: to a
# *.cso file (DXIL) when the build draws with DirectX 12, a *.spv file
# (SPIR-V) when it draws with Vulkan, or a *.glsl file when it draws with
# OpenGL (BUZ_RHI). CMake tracks the
# dependencies for us: a shader is only recompiled when its source file
# changed or when it has not been compiled yet, exactly like regular C++
# files.
#
# The shader profile is derived from the file name suffix:
#     my_shader_vs.hlsl -> vertex shader  (vs_6_0)
#     my_shader_ps.hlsl -> pixel shader   (ps_6_0)
# -------------------------------
FUNCTION(BUZ_COMPILE_SHADERS target_name)
    # Shaders live in one shared /shaders folder, so every game that calls this
    # function depends on the same "compile_shaders" target; only build it once.
    IF(NOT TARGET compile_shaders)
        FILE(GLOB shader_sources CONFIGURE_DEPENDS
             "${SOURCE_SHADER_DIRECTORY}/*.hlsl"
             "${SOURCE_ENGINE_DIRECTORY}/*/shaders/*.hlsl")

        # Stop visual studio from compiling the hlsl files with its own built-in
        # shader compiler; our shader_compiler tool does the compiling.
        SET_SOURCE_FILES_PROPERTIES(${shader_sources} PROPERTIES VS_TOOL_OVERRIDE "None")

        # A web build has no tools of its own; it runs the shader_compiler
        # of a desktop build (cmake/package.cmake passes it in).
        IF(BUZ_SHADER_COMPILER)
            SET(shader_compiler_command "${BUZ_SHADER_COMPILER}")
            SET(shader_compiler_dependency "${BUZ_SHADER_COMPILER}")
        ELSE()
            SET(shader_compiler_command shader_compiler)
            SET(shader_compiler_dependency shader_compiler)
        ENDIF()

        SET(compiled_shaders)
        FOREACH(shader_source ${shader_sources})
            GET_FILENAME_COMPONENT(shader_name ${shader_source} NAME_WE)

            IF(shader_name MATCHES "_vs$")
                SET(shader_profile "vs_6_0")
            ELSEIF(shader_name MATCHES "_ps$")
                SET(shader_profile "ps_6_0")
            ELSE()
                MESSAGE(FATAL_ERROR "Cannot derive a shader profile from \"${shader_name}.hlsl\". Shader files must end in _vs or _ps.")
            ENDIF()

            SET(shader_output_base "${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/content/shaders/${shader_name}")

            IF(BUZ_RHI STREQUAL "d3d12")
                SET(shader_extension cso)
                SET(shader_format)
                SET(shader_format_name DXIL)
            ELSEIF(BUZ_RHI STREQUAL "vulkan")
                SET(shader_extension spv)
                SET(shader_format --spirv)
                SET(shader_format_name SPIR-V)
            ELSEIF(BUZ_RHI STREQUAL "opengl" AND EMSCRIPTEN)
                SET(shader_extension glsl)
                SET(shader_format --glsl=300es)
                SET(shader_format_name "GLSL ES")
            ELSEIF(BUZ_RHI STREQUAL "opengl")
                SET(shader_extension glsl)
                SET(shader_format --glsl=330)
                SET(shader_format_name GLSL)
            ELSE()
                MESSAGE(FATAL_ERROR "No shader format for BUZ_RHI \"${BUZ_RHI}\".")
            ENDIF()

            ADD_CUSTOM_COMMAND(
                OUTPUT ${shader_output_base}.${shader_extension}
                COMMAND ${shader_compiler_command} ${shader_source} ${shader_output_base}.${shader_extension} ${shader_profile} ${shader_format}
                DEPENDS ${shader_source} ${shader_compiler_dependency}
                COMMENT "Compiling shader ${shader_name}.hlsl (${shader_profile}, ${shader_format_name})"
                VERBATIM)
            LIST(APPEND compiled_shaders ${shader_output_base}.${shader_extension})
        ENDFOREACH()

        ADD_CUSTOM_TARGET(compile_shaders DEPENDS ${compiled_shaders} SOURCES ${shader_sources})
        SET_PROPERTY(TARGET compile_shaders PROPERTY FOLDER "events")
    ENDIF()

    ADD_DEPENDENCIES(${target_name} compile_shaders)
ENDFUNCTION()

#pragma once

#include <cstdint>
#include <span>
#include <string>

//--------------------------------------------------------------
/// @brief The two kinds of shader a pipeline is made of.
enum class shader_stage
{
	vertex,
	pixel,
};

//--------------------------------------------------------------
/// @brief Which GLSL a shader is translated to.
enum class glsl_version
{
	gl_330, // OpenGL 3.3 core (desktop)
	es_300, // OpenGL ES 3.0, which is what WebGL 2 runs (the browser)
};

//--------------------------------------------------------------
/// @brief Translates a shader dxc compiled to SPIR-V into GLSL, with
/// SPIRV-Cross (https://github.com/KhronosGroup/SPIRV-Cross).
///
/// GLSL 3.30 has no binding numbers, so the resources get fixed names that
/// the OpenGL backend looks up instead:
/// - the per-draw constants: the uniform block buz_constants
/// - the texture and its sampler, combined: buz_texture
/// - what the vertex shader hands the pixel shader: buz_varying_<location>
/// @param spirv The SPIR-V words.
/// @param stage What kind of shader it is.
/// @param version The GLSL to write.
/// @param out_glsl Receives the GLSL source.
/// @return False when SPIRV-Cross refused the shader (the reason is printed).
bool translate_to_glsl(std::span<const uint32_t> spirv, shader_stage stage, glsl_version version, std::string& out_glsl);

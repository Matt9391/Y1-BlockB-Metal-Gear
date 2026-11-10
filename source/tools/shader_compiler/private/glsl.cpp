#include "glsl.h"

#include <spirv_glsl.hpp>

#include <cstdio>
#include <cstring>

namespace
{
	//--------------------------------------------------------------
	/// @brief Names what the vertex shader hands the pixel shader after its
	/// location. GLSL 3.30 matches these by name, and SPIRV-Cross would call
	/// them out_var_* on one side and in_var_* on the other.
	void name_varyings(spirv_cross::CompilerGLSL& compiler, const spirv_cross::SmallVector<spirv_cross::Resource>& varyings)
	{
		for (const spirv_cross::Resource& varying : varyings)
		{
			const uint32_t location = compiler.get_decoration(varying.id, spv::DecorationLocation);
			compiler.set_name(varying.id, "buz_varying_" + std::to_string(location));
		}
	}
}

//--------------------------------------------------------------
bool translate_to_glsl(std::span<const uint32_t> spirv, shader_stage stage, glsl_version version, std::string& out_glsl)
{
	try
	{
		spirv_cross::CompilerGLSL compiler(spirv.data(), spirv.size());

		spirv_cross::CompilerGLSL::Options options;
		options.version = version == glsl_version::es_300 ? 300 : 330;
		options.es = version == glsl_version::es_300;
		options.enable_420pack_extension = false; // no binding numbers: the backend binds by name
		options.emit_push_constant_as_uniform_buffer = true;

		// OpenGL ES pixel shaders default to medium precision, which is
		// too coarse for texture coordinates on large textures
		options.fragment.default_float_precision = spirv_cross::CompilerGLSL::Options::Highp;
		options.fragment.default_int_precision = spirv_cross::CompilerGLSL::Options::Highp;

		// The engine's matrices map depth to 0..1, as DirectX and Vulkan
		// want it; OpenGL wants -1..1, so the vertex shader converts.
		options.vertex.fixup_clipspace = stage == shader_stage::vertex;
		compiler.set_common_options(options);

		const spirv_cross::ShaderResources resources = compiler.get_shader_resources();
		for (const spirv_cross::Resource& constants : resources.push_constant_buffers)
		{
			compiler.set_name(constants.base_type_id, "buz_constants");
		}

		if (stage == shader_stage::vertex)
		{
			name_varyings(compiler, resources.stage_outputs);
		}
		else
		{
			name_varyings(compiler, resources.stage_inputs);
		}

		// GLSL samples a texture and its sampler as one object
		compiler.build_combined_image_samplers();
		for (const spirv_cross::CombinedImageSampler& combined : compiler.get_combined_image_samplers())
		{
			compiler.set_name(combined.combined_id, "buz_texture");
		}

		out_glsl = compiler.compile();

		// hlsl matrices reach SPIR-V as row-major. For constants it turns
		// into a uniform block, SPIRV-Cross already undoes that by flipping
		// every multiplication (m * v becomes v * m), and still declares the
		// matrix layout(row_major), which would undo it a second time.
		for (size_t found = out_glsl.find("layout(row_major) "); found != std::string::npos; found = out_glsl.find("layout(row_major) "))
		{
			out_glsl.erase(found, std::strlen("layout(row_major) "));
		}

		// OpenGL counts pixel rows (gl_FragCoord.y) from the bottom of the
		// window, the other apis from the top. Desktop GLSL can be told to
		// count from the top, so a pixel shader that reads the position
		// gives the same answer everywhere (the PS1 dither pattern does).
		if (stage == shader_stage::pixel && version == glsl_version::gl_330 && out_glsl.find("gl_FragCoord") != std::string::npos)
		{
			const size_t after_version = out_glsl.find('\n') + 1;
			out_glsl.insert(after_version, "layout(origin_upper_left) in vec4 gl_FragCoord;\n");
		}
		return true;
	}
	catch (const spirv_cross::CompilerError& error)
	{
		std::fprintf(stderr, "shader_compiler: SPIRV-Cross cannot translate the shader to GLSL: %s\n", error.what());
		return false;
	}
}

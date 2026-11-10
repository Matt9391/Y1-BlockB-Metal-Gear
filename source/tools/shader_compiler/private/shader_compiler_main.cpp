//--------------------------------------------------------------
// shader_compiler
//
// Compiles a single hlsl source file with the directx shader compiler
// (dxc). Graphics apis only load compiled shaders, so this tool runs as
// part of the build: the build system invokes it for every shader that
// changed since the last build (see BUZ_COMPILE_SHADERS in
// cmake/compile_shaders.cmake).
//
// The same hlsl compiles to every format buz needs:
//     DXIL   (*.cso)  for the DirectX 12 backend
//     SPIR-V (*.spv)  for the Vulkan backend, with --spirv
//     GLSL   (*.glsl) for the OpenGL backend, with --glsl=330 (desktop) or
//                     --glsl=300es (the browser): dxc compiles to SPIR-V,
//                     then SPIRV-Cross turns that into GLSL (see glsl.h)
//
// Usage:
//     shader_compiler <input.hlsl> <output> <profile> [--spirv | --glsl=330 | --glsl=300es]
//
// Example:
//     shader_compiler sprite_vs.hlsl sprite_vs.spv vs_6_0 --spirv
//--------------------------------------------------------------

#if defined(_WIN32)
	// dxcapi.h needs the full com headers, so no WIN32_LEAN_AND_MEAN here
	#include <windows.h>
#endif

// On Linux dxcapi.h brings WinAdapter.h, which stands in for the bits of
// com it uses (IUnknown, HRESULT, ...), so the code below is the same on both.
#include <dxcapi.h>

#if defined(BUZ_SHADER_GLSL)
	#include "glsl.h"
#endif

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <string>
#include <string_view>
#include <vector>

namespace
{
	//--------------------------------------------------------------
	/// @brief Owns a reference to a dxc (com) object and releases it when
	/// it goes out of scope, like Microsoft::WRL::ComPtr but portable.
	template<typename T>
	class com_ptr
	{
	public:
		com_ptr() = default;
		~com_ptr()
		{
			if (m_pointer != nullptr) { m_pointer->Release(); }
		}

		com_ptr(const com_ptr&) = delete;
		com_ptr& operator=(const com_ptr&) = delete;

		T* operator->() const { return m_pointer; }
		T* get() const { return m_pointer; }

		//----------------------------------------------------------
		/// @brief Address to receive a new object into (the object held
		/// before, if any, is released first).
		T** put()
		{
			if (m_pointer != nullptr)
			{
				m_pointer->Release();
				m_pointer = nullptr;
			}
			return &m_pointer;
		}

	private:
		T* m_pointer = nullptr;
	};

	//--------------------------------------------------------------
	/// @brief What the shader is compiled to.
	enum class output_format
	{
		dxil,
		spirv,
		glsl_330,
		glsl_es_300,
	};

	//--------------------------------------------------------------
	/// @brief Reads the optional format argument.
	/// @return False when the argument names no format.
	bool parse_format(std::string_view argument, output_format& out_format)
	{
		if (argument == "--spirv") { out_format = output_format::spirv; }
		else if (argument == "--glsl=330") { out_format = output_format::glsl_330; }
		else if (argument == "--glsl=300es") { out_format = output_format::glsl_es_300; }
		else
		{
			return false;
		}
		return true;
	}

	//--------------------------------------------------------------
	/// @brief Converts a regular (ascii) string to the wide string dxc expects.
	std::wstring to_wide(const std::string& text)
	{
		return std::wstring(text.begin(), text.end());
	}

	//--------------------------------------------------------------
	/// @brief Reads a whole file into a byte buffer.
	/// @return True when the file could be read.
	bool read_file(const std::filesystem::path& path, std::vector<char>& out_data)
	{
		std::ifstream file(path, std::ios::binary | std::ios::ate);
		if (!file.is_open())
		{
			return false;
		}

		const std::streamsize size = file.tellg();
		file.seekg(0, std::ios::beg);

		out_data.resize(static_cast<size_t>(size));
		file.read(out_data.data(), size);

		return true;
	}

	//--------------------------------------------------------------
	/// @brief Writes a byte buffer to a file, creating directories when needed.
	/// @return True when the file could be written.
	bool write_file(const std::filesystem::path& path, const void* data, size_t size)
	{
		std::error_code error;
		std::filesystem::create_directories(path.parent_path(), error);

		std::ofstream file(path, std::ios::binary);
		if (!file.is_open())
		{
			return false;
		}

		file.write(static_cast<const char*>(data), static_cast<std::streamsize>(size));
		return true;
	}
}

//--------------------------------------------------------------
int main(int argument_count, char* arguments[])
{
	if (argument_count < 4)
	{
		std::printf("Usage: shader_compiler <input.hlsl> <output> <profile> [--spirv | --glsl=330 | --glsl=300es]\n");
		std::printf("Example: shader_compiler sprite_vs.hlsl sprite_vs.spv vs_6_0 --spirv\n");
		return 1;
	}

	const std::filesystem::path input_path = arguments[1];
	const std::filesystem::path output_path = arguments[2];
	const std::string profile_name = arguments[3];
	const std::wstring profile = to_wide(profile_name);

	output_format format = output_format::dxil;
	if (argument_count > 4 && !parse_format(arguments[4], format))
	{
		std::fprintf(stderr, "shader_compiler: unknown format \"%s\"\n", arguments[4]);
		return 1;
	}

	// GLSL starts out as SPIR-V
	const bool spirv = format != output_format::dxil;
	const bool glsl = format == output_format::glsl_330 || format == output_format::glsl_es_300;
#if !defined(BUZ_SHADER_GLSL)
	if (glsl)
	{
		std::fprintf(stderr, "shader_compiler: this build cannot write GLSL; configure it with BUZ_RHI=opengl\n");
		return 1;
	}
#endif

	std::vector<char> source;
	if (!read_file(input_path, source))
	{
		std::fprintf(stderr, "shader_compiler: cannot open input file \"%s\"\n", input_path.string().c_str());
		return 1;
	}

	// Create the dxc compiler
	com_ptr<IDxcCompiler3> compiler;
	if (FAILED(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(compiler.put()))))
	{
		std::fprintf(stderr, "shader_compiler: failed to create the dxc compiler\n");
		return 1;
	}

	const std::wstring input_name = input_path.filename().wstring();

	// Command line arguments for dxc, the same you would pass to dxc.exe
	std::vector<const wchar_t*> compile_arguments = {
		input_name.c_str(), // shown in error messages
		L"-T",
		profile.c_str(), // target profile (vs_6_0, ps_6_0, ...)
		L"-E",
		L"main", // entry point function
		L"-O3",  // full optimization
		L"-WX",  // treat warnings as errors
	};

	if (spirv)
	{
		compile_arguments.push_back(L"-spirv");
		compile_arguments.push_back(L"-fspv-target-env=vulkan1.3");

		// Vulkan's clip space has y pointing down where DirectX (and
		// OpenGL) have it pointing up. Flipping the position the vertex
		// shader outputs makes the same shader and the same matrices work
		// for both.
		if (!glsl && profile_name.starts_with("vs_"))
		{
			compile_arguments.push_back(L"-fvk-invert-y");
		}

		// Lets a shader work around what OpenGL ES (the browser) lacks
		if (format == output_format::glsl_es_300)
		{
			compile_arguments.push_back(L"-D");
			compile_arguments.push_back(L"BUZ_GLSL_ES=1");
		}
	}
	else
	{
		// The shaders carry [[vk::...]] annotations for the Vulkan backend.
		// DXIL has no use for them and warns that it ignores them; that is
		// expected, so do not let -WX turn it into an error.
		compile_arguments.push_back(L"-Wno-ignored-attributes");
	}

	DxcBuffer source_buffer = {};
	source_buffer.Ptr = source.data();
	source_buffer.Size = source.size();
	source_buffer.Encoding = DXC_CP_UTF8;

	com_ptr<IDxcResult> result;
	if (FAILED(compiler->Compile(&source_buffer, compile_arguments.data(), static_cast<UINT32>(compile_arguments.size()), nullptr, IID_PPV_ARGS(result.put()))))
	{
		std::fprintf(stderr, "shader_compiler: the compile call itself failed\n");
		return 1;
	}

	// Print compiler errors and warnings, if any
	com_ptr<IDxcBlobUtf8> errors;
	result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(errors.put()), nullptr);
	if (errors.get() != nullptr && errors->GetStringLength() > 0)
	{
		std::fprintf(stderr, "%s", errors->GetStringPointer());
	}

	HRESULT compile_status = S_OK;
	result->GetStatus(&compile_status);
	if (FAILED(compile_status))
	{
		std::fprintf(stderr, "shader_compiler: failed to compile \"%s\"\n", input_path.string().c_str());
		return 1;
	}

	// Write the compiled bytecode to the output file
	com_ptr<IDxcBlob> bytecode;
	if (FAILED(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(bytecode.put()), nullptr)))
	{
		std::fprintf(stderr, "shader_compiler: no bytecode was produced\n");
		return 1;
	}

	const void* output = bytecode->GetBufferPointer();
	size_t output_size = bytecode->GetBufferSize();

#if defined(BUZ_SHADER_GLSL)
	std::string glsl_source;
	if (glsl)
	{
		const std::span<const uint32_t> words(static_cast<const uint32_t*>(output), output_size / sizeof(uint32_t));
		const shader_stage stage = profile_name.starts_with("vs_") ? shader_stage::vertex : shader_stage::pixel;
		const glsl_version version = format == output_format::glsl_es_300 ? glsl_version::es_300 : glsl_version::gl_330;
		if (!translate_to_glsl(words, stage, version, glsl_source))
		{
			return 1;
		}
		output = glsl_source.data();
		output_size = glsl_source.size();
	}
#endif

	if (!write_file(output_path, output, output_size))
	{
		std::fprintf(stderr, "shader_compiler: cannot write output file \"%s\"\n", output_path.string().c_str());
		return 1;
	}

	std::printf("shader_compiler: %s -> %s (%zu bytes)\n", input_path.string().c_str(), output_path.string().c_str(), output_size);
	return 0;
}

#pragma once

#include "core/filesystem.h"

#include <cstddef>
#include <span>
#include <string>
#include <unordered_map>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Loads compiled shaders from disk and keeps them in memory.
	///
	/// Graphics apis can only work with shaders that are already compiled:
	/// DirectX 12 reads DXIL (*.cso files), Vulkan reads SPIR-V (*.spv).
	/// Our shader_compiler tool compiles every *.hlsl file in the /shaders
	/// folder during the build and stores the result next to the executable
	/// in content/shaders. This class loads those files.
	class shader_library
	{
	public:
		//----------------------------------------------------------
		/// @brief Sets where the compiled shaders are loaded from.
		/// @param shader_directory Directory containing the compiled shaders.
		/// @param extension Extension of the compiled shaders the graphics
		/// backend reads, e.g. ".cso".
		/// @return True when the directory exists.
		bool initialize(const fs::path& shader_directory, const std::string& extension);

		//----------------------------------------------------------
		/// @brief Returns the bytecode of a compiled shader, loading it on first use.
		/// @param name Shader name without extension (e.g. "sprite_vs").
		/// @return Shader bytecode, or an empty span when the shader could not be loaded.
		std::span<const std::byte> get_shader(const std::string& name);

	private:
		fs::path m_shader_directory; // where the compiled shaders are
		std::string m_extension;     // of the files the backend reads, e.g. ".cso"

		// Keeps the shader bytes alive for as long as the library exists;
		// pipeline creation reads directly from these buffers.
		std::unordered_map<std::string, std::vector<std::byte>> m_shaders;
	};
}

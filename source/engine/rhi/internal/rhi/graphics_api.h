#pragma once

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief The graphics apis buz has a backend for.
	enum class graphics_api
	{
		d3d12,  // Windows only
		vulkan, // Windows and Linux
		opengl, // Windows and Linux (OpenGL 3.3)
	};

	//--------------------------------------------------------------
	/// @brief Returns the graphics api this build draws with: the
	/// backend CMake's BUZ_RHI option compiled in.
	graphics_api get_graphics_api();
}

#pragma once

namespace buz
{
	namespace rhi
	{
		class device;
	}

	class shader_library;
	class texture_loader;

	//--------------------------------------------------------------
	/// @brief The shared rendering systems a renderer or loader needs to
	/// create its gpu resources. The engine owns them and hands this out;
	/// game code never needs it. Drawing goes through the frame's command
	/// list instead, which the engine hands to the renderers for each frame.
	struct render_context
	{
		rhi::device* device = nullptr;

		shader_library* shaders = nullptr;
		texture_loader* textures = nullptr;
	};
}

#pragma once

namespace buz
{
	namespace rhi
	{
		class command_list;
		class device;
	}

	class shader_library;
	class texture_loader;

	//--------------------------------------------------------------
	/// @brief The shared rendering systems a renderer needs to create its
	/// gpu resources, and the frame it draws into. The engine owns them and
	/// hands this out; game code never needs it.
	struct render_context
	{
		rhi::device* device = nullptr;

		shader_library* shaders = nullptr;
		texture_loader* textures = nullptr;

		// The command list of the world pass while game::render_world runs,
		// nullptr the rest of the frame: the 3D renderer draws into it.
		rhi::command_list* world_commands = nullptr;
	};
}

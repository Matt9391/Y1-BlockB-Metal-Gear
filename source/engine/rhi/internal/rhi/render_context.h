#pragma once

#include "rhi/settings.h"

#include <cstdint>

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
	/// @brief Whether meshes are drawn as the edges of their triangles only.
	using wireframe_view = utl::yes_no_t<struct wireframe_view_tag>;

	//--------------------------------------------------------------
	/// @brief Whether surfaces are lit, or all drawn at their plain color.
	using lighting_view = utl::yes_no_t<struct lighting_view_tag>;

	//--------------------------------------------------------------
	/// @brief Whether debug lines are drawn.
	using debug_lines_view = utl::yes_no_t<struct debug_lines_view_tag>;

	//--------------------------------------------------------------
	/// @brief The debug views the 3D renderer draws with. The engine switches
	/// them with its debug keys (engine_settings::debug_controls), so a game
	/// never sees them.
	struct debug_views
	{
		wireframe_view wireframe = wireframe_view::no;        // triangle edges only
		lighting_view lighting = lighting_view::yes;          // off: every surface at its plain color
		debug_lines_view debug_lines = debug_lines_view::yes; // the lines of debug_draw
	};

	//--------------------------------------------------------------
	/// @brief The shared rendering systems a renderer needs to create its
	/// gpu resources, and the frame it draws into. The engine owns them and
	/// hands this out; game code never needs it.
	struct render_context
	{
		rhi::device* device = nullptr;

		shader_library* shaders = nullptr;
		texture_loader* textures = nullptr;

		// The frame's command list while the game draws (the world, then the
		// screen), nullptr the rest of the frame: the renderers draw into it.
		rhi::command_list* commands = nullptr;

		// Counts the frames the engine drew, starting at 1: a renderer that
		// sees a new number starts its pass for the new frame.
		uint64_t frame = 0;

		// How the 2D renderers sample their textures (engine_settings).
		texture_filter sampler_filter = texture_filter::linear;

		// What the 3D renderer shows, switched by the engine's debug keys.
		debug_views views;
	};
}

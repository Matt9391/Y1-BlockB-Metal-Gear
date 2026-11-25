#pragma once

#include <yes_no.h>

namespace buz::rhi
{
	// The rhi's own words for what the engine and the renderers choose.
	// Games choose with the engine's types (engine_settings.h,
	// render_core/texture_filter.h), and the engine and renderers turn them
	// into these, so the rhi depends on nothing above it.

	//--------------------------------------------------------------
	/// @brief Which graphics card to pick when a machine has more than one.
	/// OpenGL cannot choose: it uses the card the driver picks.
	enum class adapter_preference
	{
		highest_score, // the most capable graphics card
		lowest_score,  // the least capable graphics card
		software,      // a software rasterizer (WARP, lavapipe)
	};

	//--------------------------------------------------------------
	/// @brief How a pipeline's sampler filters textures.
	enum class sampler_filter
	{
		linear,  // blends between texels
		nearest, // takes the nearest texel
	};

	// Declared here first: "struct vsync_tag" inside the template argument
	// would otherwise find buz::vsync_tag (engine_settings.h) in files that
	// include both headers, and rhi::vsync would be a different type there
	// than in the other files (a One Definition Rule violation)
	struct vsync_tag;

	//--------------------------------------------------------------
	/// @brief Whether presenting a frame waits for the monitor's refresh.
	using vsync = utl::yes_no_t<vsync_tag>;
}

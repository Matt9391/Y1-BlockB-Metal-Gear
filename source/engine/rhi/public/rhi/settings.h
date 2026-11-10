/// @file
/// @brief The graphics settings games choose: texture filtering, the graphics card, vsync.

#pragma once

#include <yes_no.h>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Which graphics card the engine should pick when a machine has
	/// more than one. OpenGL cannot choose: it uses the card the driver picks.
	enum class adapter_preference
	{
		highest_score, // the most capable graphics card (default)
		lowest_score,  // the least capable graphics card, useful to test low-end behavior
		software,      // a software rasterizer (WARP, lavapipe): slow, but needs no graphics card; for automated tests
	};

	//--------------------------------------------------------------
	/// @brief How sampled textures are filtered when scaled on screen.
	enum class texture_filter
	{
		linear,  // smooth blending between texels, blurs pixel art when scaled (default)
		nearest, // no blending, keeps hard pixel edges when scaled (pixel art look)
	};

	//--------------------------------------------------------------
	/// @brief Whether presenting a frame waits for the monitor's refresh
	/// (no tearing, frame rate capped at the refresh rate) or not.
	using vsync = utl::yes_no_t<struct vsync_tag>;
}

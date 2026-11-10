/// @file
/// @brief The settings the engine starts with.

#pragma once

#include "core/math.h"
#include "rhi/settings.h"

#include <yes_no.h>

#include <cstdint>
#include <string>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Whether the stats overlay is visible.
	using stats_overlay = utl::yes_no_t<struct stats_overlay_tag>;

	//--------------------------------------------------------------
	/// @brief Settings the engine starts with.
	///
	/// Set the ones you want to change and keep the rest; designated
	/// initializers read well for that:
	/// @code{.cpp}
	/// buz::engine engine(argc, argv, { .window_title = "my game", .sampler_filter = buz::texture_filter::nearest });
	/// @endcode
	/// The most common ones can also be changed at launch, without
	/// rebuilding; the command line wins over the game's settings:
	///
	/// | setting | example |
	/// |---|---|
	/// | `-window_width`, `-window_height` | `-window_width=1920` |
	/// | `-window_title` | `-window_title=Demo` |
	/// | `-gpu` | `-gpu=lowest`, or `-gpu=software` for a software rasterizer |
	/// | `-screenshot`, `-screenshot_frame` | `-screenshot=frame.png -screenshot_frame=300` |
	///
	/// A setting nothing understands is logged as a warning, so a typo
	/// shows up instead of being silently ignored.
	struct engine_settings
	{
		/// @brief Width of the window's drawable area in pixels. The window
		/// cannot be resized once it is open.
		uint32_t window_width = 1280;

		/// @brief Height of the window's drawable area in pixels.
		uint32_t window_height = 720;

		/// @brief Text in the window's title bar and on the taskbar.
		std::string window_title = "BuzEngine";

		/// @brief Which graphics card to run on; lowest_score is handy to
		/// test how the game behaves on weaker hardware, software runs
		/// without one.
		adapter_preference gpu_preference = adapter_preference::highest_score;

		/// @brief The background: the color every frame starts with before
		/// anything is drawn.
		color clear_color = { 0.1f, 0.1f, 0.15f, 1.0f };

		/// @brief How textures are filtered when scaled on screen; nearest
		/// keeps pixel art crisp instead of blurring it like linear does.
		texture_filter sampler_filter = texture_filter::linear;

		/// @brief Wait for the monitor between frames: no tearing, and the
		/// frame rate stays at the monitor's refresh rate.
		vsync use_vsync = vsync::yes;

		/// @brief Show the stats overlay (frame rate, draw calls) from the
		/// start. A debug key switches it while the game runs.
		stats_overlay show_stats = stats_overlay::no;

		/// @brief When set, the engine saves frame screenshot_frame as a png
		/// at this path and quits. Until then every frame lasts exactly
		/// 1/60 s and the keyboard and mouse are ignored, so every run, and
		/// every graphics api, draws the very same frame: the way to compare
		/// two versions pixel by pixel. Relative paths end up in the game's
		/// saved/<game> folder next to the executable.
		std::string screenshot_path;

		/// @brief The frame a screenshot run saves (see screenshot_path),
		/// counted from 0.
		int screenshot_frame = 120;
	};
}

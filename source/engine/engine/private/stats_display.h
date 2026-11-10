#pragma once

#include "engine/engine_settings.h"
#include "renderer_2d/font.h"

#include <memory>

namespace buz
{
	namespace rhi
	{
		struct frame_stats;
	}

	class screen_renderer;
	class texture;

	//--------------------------------------------------------------
	/// @brief The stats overlay: the frame rate and what the gpu was asked to
	/// do, in the top-right corner, on top of everything.
	class stats_display
	{
	public:
		//----------------------------------------------------------
		/// @brief Starts shown or hidden, as the settings say.
		explicit stats_display(stats_overlay visible);

		//----------------------------------------------------------
		/// @brief Sets the font atlas the numbers are drawn with.
		void set_font(std::shared_ptr<texture> atlas);

		//----------------------------------------------------------
		/// @brief Shows the overlay when it is hidden, and hides it when shown.
		void toggle();

		//----------------------------------------------------------
		/// @brief Draws the overlay, when it is shown.
		/// @param screen Renderer of the screen pass.
		/// @param stats What the gpu was asked to do in the previous frame.
		/// @param frame_seconds How long the frame took.
		/// @param window_width Width of the window in pixels.
		void draw(screen_renderer& screen, const rhi::frame_stats& stats, float frame_seconds, float window_width);

	private:
		font m_font;                    // the numbers are drawn with it
		stats_overlay m_visible;        // switched by a debug key
		float m_smoothed_frame_seconds; // smoothed so the numbers stay readable
	};
}

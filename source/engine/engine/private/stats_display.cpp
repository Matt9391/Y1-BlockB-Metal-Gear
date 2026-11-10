#include "stats_display.h"

#include "renderer_2d/screen_renderer.h"
#include "rhi/device.h"

#include <format>
#include <string>

namespace buz
{
	//--------------------------------------------------------------
	stats_display::stats_display(stats_overlay visible)
		: m_visible(visible)
		, m_smoothed_frame_seconds(0.0f)
	{
	}

	//--------------------------------------------------------------
	void stats_display::set_font(std::shared_ptr<texture> atlas)
	{
		m_font.atlas = std::move(atlas);
	}

	//--------------------------------------------------------------
	void stats_display::toggle()
	{
		m_visible = m_visible ? stats_overlay::no : stats_overlay::yes;
	}

	//--------------------------------------------------------------
	void stats_display::draw(screen_renderer& screen, const rhi::frame_stats& stats, float frame_seconds, float window_width)
	{
		if (!m_visible)
		{
			return;
		}

		m_smoothed_frame_seconds = glm::mix(m_smoothed_frame_seconds, frame_seconds, 0.05f);
		const float frame_ms = m_smoothed_frame_seconds * 1000.0f;
		const float fps = m_smoothed_frame_seconds > 0.0f ? 1.0f / m_smoothed_frame_seconds : 0.0f;

		// The stats are the previous frame's: this frame is still being
		// recorded. They include the overlay's own draw calls (one per character).
		const std::string text = std::format("{:.0f} fps  {:.2f} ms\ndraw calls {}\ninstances  {}", fps, frame_ms, stats.draw_calls, stats.instances);

		constexpr float k_pixel_height = 16.0f;
		constexpr float k_margin = 12.0f;
		constexpr float k_width = 22.0f * k_pixel_height; // the longest line fits in 22 characters
		constexpr float k_border = 6.0f;
		constexpr color k_background = { 0.0f, 0.0f, 0.0f, 0.6f };
		constexpr color k_text_color = { 0.6f, 1.0f, 0.6f, 1.0f };

		const vec2 top_left = { window_width - k_width - k_margin, k_margin };
		const vec2 background_min = top_left - vec2(k_border, k_border);
		const vec2 background_max = top_left + vec2(k_width + k_border, 3.0f * k_pixel_height + k_border);
		screen.draw_rect({ background_min, background_max }, k_background);
		screen.draw_text(m_font, text, top_left, k_pixel_height, k_text_color);
	}
}

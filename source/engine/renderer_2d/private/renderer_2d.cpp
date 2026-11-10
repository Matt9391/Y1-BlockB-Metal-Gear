#include "renderer_2d/renderer_2d.h"

#include "renderer_2d/font.h"
#include "renderer_2d/sprite_pass.h"
#include "rhi/texture.h"

#include <cmath>

namespace
{
	//--------------------------------------------------------------
	/// @brief Circles are drawn as a fan of this many triangles.
	constexpr int k_circle_segments = 48;
}

namespace buz
{
	//--------------------------------------------------------------
	renderer_2d::renderer_2d(sprite_pass& pass)
		: m_pass(pass)
	{
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_sprite(const texture& sprite_texture, vec2 position, vec2 size, float rotation_radians, color tint)
	{
		// No source rect given, so the uv rect covers the whole texture
		m_pass.submit_quad(sprite_texture, { 0.0f, 0.0f, 1.0f, 1.0f }, position, size, rotation_radians, tint);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_sprite(const texture& sprite_texture, rect source_rect, vec2 position, vec2 size, float rotation_radians, color tint)
	{
		// The source rect is in texture pixels; the shader wants it in
		// 0..1 texture space.
		const vec2 texture_size = sprite_texture.get_size();

		const vec2 uv_offset = source_rect.min / texture_size;
		const vec2 uv_scale = source_rect.size() / texture_size;

		m_pass.submit_quad(sprite_texture, { uv_offset.x, uv_offset.y, uv_scale.x, uv_scale.y }, position, size, rotation_radians, tint);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_rect(rect area, color fill_color)
	{
		// A rectangle is just the white texture stretched and tinted
		m_pass.submit_quad(m_pass.get_white_texture(), { 0.0f, 0.0f, 1.0f, 1.0f }, area.center(), area.size(), 0.0f, fill_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_rect_outline(rect area, float thickness, color outline_color)
	{
		// Four thin bars along the edges: top, bottom, left, right. The
		// left and right bars are inset so the corners are not drawn twice
		// (which would show with transparent colors).
		draw_rect({ area.min, { area.max.x, area.min.y + thickness } }, outline_color);
		draw_rect({ { area.min.x, area.max.y - thickness }, area.max }, outline_color);
		draw_rect({ { area.min.x, area.min.y + thickness }, { area.min.x + thickness, area.max.y - thickness } }, outline_color);
		draw_rect({ { area.max.x - thickness, area.min.y + thickness }, area.max - vec2(0.0f, thickness) }, outline_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_line(vec2 from, vec2 to, float thickness, color line_color)
	{
		// A line is a long thin quad: as wide as the distance between the
		// two points, rotated to point from one to the other.
		const vec2 difference = to - from;
		const float line_length = length(difference);
		const float angle = std::atan2(difference.y, difference.x);
		const vec2 line_center = (from + to) * 0.5f;

		m_pass.submit_quad(m_pass.get_white_texture(), { 0.0f, 0.0f, 1.0f, 1.0f }, line_center, { line_length, thickness }, angle, line_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_circle(vec2 center, float radius, color fill_color)
	{
		// A fan of thin triangles around the center, each connecting the
		// center with two neighbouring points on the edge.
		vec2 corners[k_circle_segments * 3];
		for (int segment = 0; segment < k_circle_segments; ++segment)
		{
			const float angle_from = 2.0f * k_pi * static_cast<float>(segment) / static_cast<float>(k_circle_segments);
			const float angle_to = 2.0f * k_pi * static_cast<float>(segment + 1) / static_cast<float>(k_circle_segments);

			corners[segment * 3 + 0] = center;
			corners[segment * 3 + 1] = center + vec2(std::cos(angle_from), std::sin(angle_from)) * radius;
			corners[segment * 3 + 2] = center + vec2(std::cos(angle_to), std::sin(angle_to)) * radius;
		}

		m_pass.submit_triangles(corners, fill_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_triangle(vec2 a, vec2 b, vec2 c, color fill_color)
	{
		const vec2 corners[] = { a, b, c };
		m_pass.submit_triangles(corners, fill_color);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_sprite_instances(const texture& sprite_texture, std::span<const sprite_instance> instances)
	{
		m_pass.submit_instances(sprite_texture, instances);
	}

	//--------------------------------------------------------------
	void renderer_2d::draw_text(const font& text_font, std::string_view text, vec2 position, float pixel_height, color tint)
	{
		// Scale the glyphs so they are pixel_height tall on screen
		const float scale = pixel_height / static_cast<float>(text_font.glyph_size.y);
		const vec2 glyph_screen_size = vec2(text_font.glyph_size) * scale;

		// draw_sprite positions centers, so the pen starts half a glyph in
		vec2 pen = position + glyph_screen_size * 0.5f;

		for (const char character : text)
		{
			if (character == '\n')
			{
				pen.x = position.x + glyph_screen_size.x * 0.5f;
				pen.y += glyph_screen_size.y;
				continue;
			}

			// Spaces only advance the pen, there is nothing to draw
			if (character != ' ')
			{
				draw_sprite(*text_font.atlas, text_font.get_glyph_source_rect(character), pen, glyph_screen_size, 0.0f, tint);
			}

			pen.x += glyph_screen_size.x;
		}
	}
}

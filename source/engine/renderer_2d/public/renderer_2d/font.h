/// @file
/// @brief Fixed-grid bitmap fonts for draw_text.

#pragma once

#include "core/math.h"
#include "core/rect.h"
#include "rhi/texture.h"

#include <cstdint>
#include <memory>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief A fixed-grid bitmap font: one atlas texture holding every
	/// character in a regular grid, in ascii order.
	///
	/// All glyph data lives in the atlas image (content/fonts/), this struct
	/// only describes the grid so draw_text can find each character:
	/// @code{.cpp}
	/// buz::font font;
	/// font.atlas = buz::load_texture(engine, "fonts/font_8x8.png");
	/// renderer.draw_text(font, "hello", { 20.0f, 20.0f }, 16.0f);
	/// @endcode
	///
	/// The defaults match the engine's built-in atlas
	/// (content/fonts/font_8x8.png): 8x8 pixel glyphs, 16 per row, starting
	/// at the space character (ascii 32).
	struct font
	{
		/// @brief The texture with every glyph.
		std::shared_ptr<texture> atlas;
		/// @brief The character of the first glyph, in ascii.
		uint32_t first_character = 32;
		/// @brief How many glyphs the atlas holds.
		uint32_t glyph_count = 96;
		/// @brief How many glyphs one row of the atlas holds.
		uint32_t glyphs_per_row = 16;
		/// @brief Size of one glyph in pixels.
		uvec2 glyph_size = { 8, 8 };

		//----------------------------------------------------------
		/// @brief Returns where a character's glyph sits in the atlas, in
		/// pixels. Characters the font does not have come out as the first
		/// glyph (usually space, so they show up as blank).
		rect get_glyph_source_rect(char character) const;
	};
}

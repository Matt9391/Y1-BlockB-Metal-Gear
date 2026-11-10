#include "renderer_2d/font.h"

namespace buz
{
	//--------------------------------------------------------------
	rect font::get_glyph_source_rect(char character) const
	{
		// The cast via unsigned char keeps characters above 127 from
		// going negative and ending up as a huge index.
		uint32_t index = static_cast<unsigned char>(character) - first_character;
		if (index >= glyph_count)
		{
			index = 0;
		}

		const float left = static_cast<float>((index % glyphs_per_row) * glyph_size.x);
		const float top = static_cast<float>((index / glyphs_per_row) * glyph_size.y);
		const vec2 top_left = { left, top };

		return { top_left, top_left + vec2(glyph_size) };
	}
}

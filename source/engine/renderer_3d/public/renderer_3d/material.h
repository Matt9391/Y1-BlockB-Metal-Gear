/// @file
/// @brief How a mesh looks: a color, a texture, lit or not, and which pixels are see-through.

#pragma once

#include "core/math.h"
#include "core/texture_id.h"

#include <yes_no.h>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Whether a surface is lit by the scene's light, or shows its
	/// color as is.
	using lighting = utl::yes_no_t<struct lighting_tag>;

	//--------------------------------------------------------------
	/// @brief How a mesh looks: a color, optionally times a texture, lit or
	/// not, optionally with see-through holes.
	///
	/// These are the only options the engine's shaders read; a surface that
	/// needs anything else needs shaders of its own.
	struct material
	{
		/// @brief The color, multiplied with the texture.
		color base_color;
		/// @brief The image the texture shows; none means the plain color.
		/// A model's materials name the image in the file: load it with
		/// buz::load_texture(engine, id) and hand the texture to
		/// world_renderer_3d::draw, which draws with the texture it is given.
		texture_id base_texture;
		/// @brief Whether the scene's light shades the surface.
		lighting lit = lighting::yes;
		/// @brief Pixels whose alpha (texture times color) is below this are
		/// not drawn, which leaves a hole: a railing is a flat quad whose
		/// texture has alpha 0 between the bars, cut out with 0.5. The default,
		/// 0, draws every pixel, so alpha between 0 and 1 blends instead.
		float alpha_cutoff = 0.0f;
	};
}

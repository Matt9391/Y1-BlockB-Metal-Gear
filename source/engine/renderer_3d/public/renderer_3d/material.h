/// @file
/// @brief How a mesh looks: a color, a texture, lit or not, and which pixels are see-through.

#pragma once

#include "core/math.h"

#include <yes_no.h>

#include <memory>

namespace buz
{
	class texture;

	//--------------------------------------------------------------
	/// @brief Whether a surface is lit by the scene's light, or shows its
	/// color as is.
	using lighting = utl::yes_no_t<struct lighting_tag>;

	//--------------------------------------------------------------
	/// @brief How a mesh looks: a color, optionally times a texture, lit or
	/// not, optionally with see-through holes.
	///
	/// That is all on purpose: the Block B project is about the game, not
	/// about shaders.
	struct material
	{
		/// @brief The color, multiplied with the texture.
		color base_color;
		/// @brief The texture; none means the plain color.
		std::shared_ptr<texture> base_texture;
		/// @brief Whether the scene's light shades the surface.
		lighting lit = lighting::yes;
		/// @brief Pixels whose alpha (texture times color) is below this are
		/// not drawn, which leaves a hole: a railing is a flat quad whose
		/// texture has alpha 0 between the bars, cut out with 0.5. The default,
		/// 0, draws every pixel, so alpha between 0 and 1 blends instead.
		float alpha_cutoff = 0.0f;
	};
}

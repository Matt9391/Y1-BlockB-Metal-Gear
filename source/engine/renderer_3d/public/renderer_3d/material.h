/// @file
/// @brief How a mesh looks: a color, a texture, lit or not.

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
	/// @brief How a mesh looks: a color, optionally times a texture, lit or not.
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
	};
}

/// @file
/// @brief How textures are filtered when they are drawn larger or smaller.

#pragma once

namespace buz
{
	//--------------------------------------------------------------
	/// @brief How sampled textures are filtered when scaled on screen.
	enum class texture_filter
	{
		linear,  // smooth blending between texels, blurs pixel art when scaled (default)
		nearest, // no blending, keeps hard pixel edges when scaled (pixel art look)
	};
}

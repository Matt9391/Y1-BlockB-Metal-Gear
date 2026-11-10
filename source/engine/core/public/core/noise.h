/// @file
/// @brief Smooth noise for procedural content.

#pragma once

namespace buz
{
	// Smooth noise for procedural content: terrain heights, cloud patterns,
	// camera shake, wobbly movement, ... Unlike random_float these functions
	// have no state: the same coordinates always return the same value, and
	// nearby coordinates return similar values (that is what makes the noise
	// smooth).
	//
	// The implementation is stb_perlin (source/third_party/stb).

	//--------------------------------------------------------------
	/// @brief Samples smooth 2d noise, roughly in [-1, 1].
	///
	/// The noise repeats its features about once per unit, so scale your
	/// input coordinates to control how fast the noise changes:
	/// @code{.cpp}
	/// float height = buz::noise_2d(x * 0.01f, y * 0.01f); // slow, smooth
	/// @endcode
	float noise_2d(float x, float y);

	//--------------------------------------------------------------
	/// @brief Samples smooth 3d noise, roughly in [-1, 1]. A common trick is
	/// to use time as the third coordinate to animate 2d noise.
	float noise_3d(float x, float y, float z);
}

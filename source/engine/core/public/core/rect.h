/// @file
/// @brief The 2D axis aligned rectangle and the questions to ask it.

#pragma once

#include "core/math.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief 2D axis aligned rectangle, the workhorse for collision checks.
	///
	/// Stored as two corners: min is the top-left, max is the bottom-right
	/// (remember: in screen coordinates the y axis points down).
	struct rect
	{
		/// @brief The top-left corner.
		vec2 min = { 0.0f, 0.0f };
		/// @brief The bottom-right corner.
		vec2 max = { 0.0f, 0.0f };

		//----------------------------------------------------------
		/// @brief Compares both corners; exact float equality, no epsilon.
		bool operator==(const rect&) const = default;

		//----------------------------------------------------------
		/// @brief Returns the position of the rect center.
		vec2 center() const;

		//----------------------------------------------------------
		/// @brief Returns the width and height of the rect.
		vec2 size() const;

		//----------------------------------------------------------
		/// @brief Returns true when the point lies inside the rect.
		bool contains(vec2 point) const;
	};

	//--------------------------------------------------------------
	/// @brief Builds a rect from its center and size, matching how
	/// sprites are positioned in draw_sprite.
	/// @param center Position of the rect center in pixels.
	/// @param size Width and height in pixels.
	rect rect_from_center_size(vec2 center, vec2 size);

	//--------------------------------------------------------------
	/// @brief Returns true when the two rects overlap.
	bool intersects(rect a, rect b);
}

/// @file
/// @brief The 3D axis aligned box.

#pragma once

#include "core/math.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief 3D axis aligned box: the 3D version of rect, for collision
	/// checks. Stored as its two extreme corners. The checks themselves are
	/// yours to write (see the collision guide).
	struct aabb
	{
		/// @brief The corner with the smallest x, y and z.
		vec3 min = { 0.0f, 0.0f, 0.0f };
		/// @brief The corner with the largest x, y and z.
		vec3 max = { 0.0f, 0.0f, 0.0f };

		//----------------------------------------------------------
		/// @brief Compares both corners; exact float equality, no epsilon.
		bool operator==(const aabb&) const = default;

		//----------------------------------------------------------
		/// @brief Returns the position of the box center.
		vec3 center() const;

		//----------------------------------------------------------
		/// @brief Returns the width, height and depth of the box.
		vec3 size() const;
	};

	//--------------------------------------------------------------
	/// @brief Builds a box from its center and size.
	aabb aabb_from_center_size(vec3 center, vec3 size);
}

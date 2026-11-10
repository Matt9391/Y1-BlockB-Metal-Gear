/// @file
/// @brief Debug lines that show what the code thinks is there.

#pragma once

#include "core/aabb.h"
#include "core/math.h"

namespace buz
{
	class world_renderer_3d;

	//--------------------------------------------------------------
	// Debug drawing: lines that show what your code thinks is there. Draw
	// your colliders, a guard's vision cone, a patrol path, a ray you cast,
	// and you see at a glance whether the math does what you meant.
	//
	// Lines only show while debug lines are on (world_renderer_3d::
	// toggle_debug_lines), so they can stay in the code. They are hidden
	// behind walls like everything else.
	//--------------------------------------------------------------
	namespace debug_draw
	{
		//----------------------------------------------------------
		/// @brief Draws a line between two points.
		void line(world_renderer_3d& world, vec3 from, vec3 to, color line_color);

		//----------------------------------------------------------
		/// @brief Draws the twelve edges of a box.
		void box(world_renderer_3d& world, const aabb& area, color line_color);

		//----------------------------------------------------------
		/// @brief Draws a sphere as three circles, one around each axis.
		void sphere(world_renderer_3d& world, vec3 center, float radius, color line_color);

		//----------------------------------------------------------
		/// @brief Draws a cone: what a guard (at apex, looking along
		/// direction) can see up to length away and angle to either side.
		/// @param world World renderer to draw the lines with.
		/// @param apex Tip of the cone.
		/// @param direction Where the cone points; does not need to be normalized.
		/// @param length How far the cone reaches.
		/// @param half_angle_radians Angle between the axis and the side.
		/// @param line_color Color of the lines.
		void cone(world_renderer_3d& world, vec3 apex, vec3 direction, float length, float half_angle_radians, color line_color);
	}
}

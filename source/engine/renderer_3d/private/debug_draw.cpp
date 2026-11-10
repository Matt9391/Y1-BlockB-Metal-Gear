#include "renderer_3d/debug_draw.h"

#include "renderer_3d.h"

#include <cmath>
#include <vector>

namespace buz
{
	namespace debug_draw
	{
		//--------------------------------------------------------------
		void line(world_renderer_3d& world, vec3 from, vec3 to, color line_color)
		{
			const line_vertex vertices[] = { { from, line_color }, { to, line_color } };
			world.get_renderer().draw_lines(vertices);
		}

		//--------------------------------------------------------------
		void box(world_renderer_3d& world, const aabb& area, color line_color)
		{
			const vec3 a = area.min;
			const vec3 b = area.max;
			const vec3 corners[8] = {
				{ a.x, a.y, a.z }, { b.x, a.y, a.z }, { b.x, a.y, b.z }, { a.x, a.y, b.z },
				{ a.x, b.y, a.z }, { b.x, b.y, a.z }, { b.x, b.y, b.z }, { a.x, b.y, b.z }
			};
			const int edges[12][2] = { { 0, 1 }, { 1, 2 }, { 2, 3 }, { 3, 0 }, { 4, 5 }, { 5, 6 }, { 6, 7 }, { 7, 4 }, { 0, 4 }, { 1, 5 }, { 2, 6 }, { 3, 7 } };

			line_vertex vertices[24];
			for (int edge = 0; edge < 12; ++edge)
			{
				vertices[edge * 2 + 0] = { corners[edges[edge][0]], line_color };
				vertices[edge * 2 + 1] = { corners[edges[edge][1]], line_color };
			}
			world.get_renderer().draw_lines(vertices);
		}

		// Straight lines one debug circle is drawn with, two vertices each
		constexpr int k_circle_segments = 32;
		constexpr size_t k_circle_vertices = k_circle_segments * 2;

		//--------------------------------------------------------------
		/// @brief Adds a circle around center in the plane of axis_a and axis_b.
		void add_circle(std::vector<line_vertex>& vertices, vec3 center, vec3 axis_a, vec3 axis_b, float radius, color line_color)
		{
			for (int segment = 0; segment < k_circle_segments; ++segment)
			{
				const float angle_from = 2.0f * k_pi * static_cast<float>(segment) / static_cast<float>(k_circle_segments);
				const float angle_to = 2.0f * k_pi * static_cast<float>(segment + 1) / static_cast<float>(k_circle_segments);
				vertices.push_back({ center + (axis_a * std::cos(angle_from) + axis_b * std::sin(angle_from)) * radius, line_color });
				vertices.push_back({ center + (axis_a * std::cos(angle_to) + axis_b * std::sin(angle_to)) * radius, line_color });
			}
		}

		//--------------------------------------------------------------
		void sphere(world_renderer_3d& world, vec3 center, float radius, color line_color)
		{
			std::vector<line_vertex> vertices;
			vertices.reserve(3 * k_circle_vertices); // three circles
			add_circle(vertices, center, { 1.0f, 0.0f, 0.0f }, { 0.0f, 1.0f, 0.0f }, radius, line_color);
			add_circle(vertices, center, { 0.0f, 1.0f, 0.0f }, { 0.0f, 0.0f, 1.0f }, radius, line_color);
			add_circle(vertices, center, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f, 0.0f }, radius, line_color);
			world.get_renderer().draw_lines(vertices);
		}

		//--------------------------------------------------------------
		void cone(world_renderer_3d& world, vec3 apex, vec3 direction, float length, float half_angle_radians, color line_color)
		{
			if (glm::length(direction) <= 0.0f)
			{
				return;
			}

			// Two directions at right angles to the axis span the rim circle
			const vec3 axis = normalize(direction);
			const vec3 helper = std::abs(axis.y) < 0.99f ? vec3(0.0f, 1.0f, 0.0f) : vec3(1.0f, 0.0f, 0.0f);
			const vec3 side_a = normalize(cross(axis, helper));
			const vec3 side_b = cross(axis, side_a);

			const vec3 rim_center = apex + axis * length;
			const float rim_radius = length * std::tan(half_angle_radians);

			std::vector<line_vertex> vertices;
			vertices.reserve(k_circle_vertices + 4 * 2); // the rim, then four lines to the apex
			add_circle(vertices, rim_center, side_a, side_b, rim_radius, line_color);
			for (const vec3& side : { side_a, -side_a, side_b, -side_b })
			{
				vertices.push_back({ apex, line_color });
				vertices.push_back({ rim_center + side * rim_radius, line_color });
			}
			world.get_renderer().draw_lines(vertices);
		}
	}
}

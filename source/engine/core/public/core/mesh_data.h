/// @file
/// @brief Triangles on the cpu: the vertices and indices a mesh is made from.

#pragma once

#include "core/math.h"

#include <cstdint>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief One corner of a triangle of a mesh.
	struct mesh_vertex
	{
		/// @brief Where the corner is.
		vec3 position = { 0.0f, 0.0f, 0.0f };
		/// @brief The direction the surface faces, for lighting.
		vec3 normal = { 0.0f, 1.0f, 0.0f };
		/// @brief Where on the texture this corner is, 0..1.
		vec2 uv = { 0.0f, 0.0f };
	};

	//--------------------------------------------------------------
	/// @brief Triangles on the cpu, before they go to the graphics card:
	/// what buz::create_mesh uploads, and what buz::import_model reads from
	/// a file. Plain vectors, so you can build, merge or change them first.
	struct mesh_data
	{
		/// @brief The corners.
		std::vector<mesh_vertex> vertices;
		/// @brief Three per triangle, indices into vertices, counter
		/// clockwise seen from the front.
		std::vector<uint32_t> indices;
	};
}

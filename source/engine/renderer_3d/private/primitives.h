#pragma once

#include "renderer_3d/mesh.h"

#include <cstdint>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief The shapes renderer_3d can draw without a mesh file.
	enum class primitive
	{
		box,
		sphere,
		plane,
		cylinder,
		octahedron,
		count,
	};

	//--------------------------------------------------------------
	/// @brief Vertices and indices of a shape, before they go to the gpu.
	struct mesh_data
	{
		std::vector<mesh_vertex> vertices;
		std::vector<uint32_t> indices;
	};

	//--------------------------------------------------------------
	/// @brief Builds a primitive: unit sized, around the origin, with
	/// normals pointing out and triangles counter clockwise seen from outside.
	mesh_data make_primitive(primitive shape);
}

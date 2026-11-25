/// @file
/// @brief The built-in shapes as triangles: a box, a sphere, a plane, a cylinder and an octahedron.

#pragma once

#include "core/mesh_data.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief The shapes the engine can build without a model file;
	/// world_renderer_3d draws them with draw_box, draw_sphere and so on.
	enum class primitive
	{
		box,
		sphere,
		plane,
		cylinder,
		octahedron,
	};

	//--------------------------------------------------------------
	/// @brief Builds a primitive: unit sized, around the origin, with
	/// normals pointing out and triangles counter clockwise seen from
	/// outside. Upload it with buz::create_mesh to draw it with a texture,
	/// or change the vertices first.
	mesh_data make_primitive(primitive shape);
}

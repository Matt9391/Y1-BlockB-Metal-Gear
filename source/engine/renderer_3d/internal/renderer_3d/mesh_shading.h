#pragma once

#include "rhi/settings.h"

#include <string>

namespace buz
{
	class world_renderer_3d;

	//--------------------------------------------------------------
	/// @brief Which shaders draw a world_renderer_3d's meshes, and how they
	/// filter textures. For engine modules that change how the world looks
	/// (a PlayStation look, say): their shaders read the same vertices and
	/// constants as mesh_vs.hlsl and mesh_ps.hlsl.
	struct mesh_shading
	{
		std::string vertex_shader = "mesh_vs";
		std::string pixel_shader = "mesh_ps";
		texture_filter filter = texture_filter::linear;

		bool operator==(const mesh_shading& other) const = default;
	};

	//--------------------------------------------------------------
	/// @brief Draws the meshes of world with other shaders from now on.
	/// Setting the shading that is already in use does nothing, so it can be
	/// called every frame.
	void set_mesh_shading(world_renderer_3d& world, const mesh_shading& shading);
}

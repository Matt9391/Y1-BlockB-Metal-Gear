/// @file
/// @brief Drawing a world's meshes with your own shaders.

#pragma once

#include "render_core/texture_filter.h"

#include <string>

namespace buz
{
	class world_renderer_3d;

	//--------------------------------------------------------------
	/// @brief Which shaders draw a world_renderer_3d's meshes, and how they
	/// filter textures. The defaults are the built-in shaders,
	/// shaders/diffuse_vs.hlsl and shaders/diffuse_ps.hlsl. A shader is named
	/// by its file name without .hlsl; put your own next to them in /shaders.
	/// Your shaders get the same vertices and constants as the diffuse ones,
	/// so start from a copy of those, which declare the first two of these:
	/// - per draw, at register(b0): `world` (object space to world space),
	///   the rows of the normal matrix, and the material's `lit`,
	///   `alpha_cutoff` and `base_color`
	/// - per frame, at register(b2): the camera's `view_projection` (world
	///   space to the screen) and `camera_position` (in world space)
	/// - at register(b1): the data you hand to
	///   world_renderer_3d::set_shader_data
	/// @code{.cpp}
	/// buz::set_surface_shading(world, { .vertex_shader = "fog_vs", .pixel_shader = "fog_ps" });
	/// @endcode
	struct surface_shading
	{
		/// @brief The vertex shader's file name in /shaders, without ".hlsl".
		std::string vertex_shader = "diffuse_vs";
		/// @brief The pixel shader's file name in /shaders, without ".hlsl".
		std::string pixel_shader = "diffuse_ps";
		/// @brief How the meshes' textures are filtered.
		texture_filter filter = texture_filter::linear;

		bool operator==(const surface_shading& other) const = default;
	};

	//--------------------------------------------------------------
	/// @brief Draws the meshes of world with other shaders from now on.
	/// Setting the shading that is already in use does nothing, so it can be
	/// called every frame, from on_render_world for example.
	/// @param world The renderer whose meshes change.
	/// @param shading The shaders and the texture filter to draw with.
	void set_surface_shading(world_renderer_3d& world, const surface_shading& shading);
}

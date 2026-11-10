#pragma once

#include "core/math.h"
#include "rhi/pipeline.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief A single vertex of the unit quad (must match the input of
	/// sprite_vs.hlsl).
	struct sprite_vertex
	{
		vec2 position; // corner of the quad, -0.5..0.5
		vec2 uv;       // texture coordinate, 0..1 from the top-left
	};

	//--------------------------------------------------------------
	/// @brief Constants sent to the shaders for every sprite (must match
	/// sprite_vs.hlsl).
	struct sprite_constants
	{
		mat4 transform; // unit quad to the screen
		color tint;     // multiplied with the texture
		vec4 uv_rect;   // offset in xy, scale in zw (0..1 texture space)
	};

	static_assert(sizeof(sprite_constants) <= rhi::k_max_constants_size, "sprite constants no longer fit the per-draw constants");

	//--------------------------------------------------------------
	/// @brief One sprite as the instanced vertex shader reads it (must
	/// match the per-instance inputs of sprite_instanced_vs.hlsl).
	struct gpu_sprite_instance
	{
		vec4 position_size; // center in xy, size in zw
		vec4 uv_rect;       // offset in xy, scale in zw (0..1 texture space)
		color tint;         // multiplied with the texture
		vec4 rotation;      // radians in x
	};

	static_assert(sizeof(gpu_sprite_instance) == 4 * sizeof(vec4), "renderer_2d::m_instance_data stores instances as four vec4");

	//--------------------------------------------------------------
	/// @brief Constants of the instanced pipeline: only the camera, every
	/// other value comes per instance.
	struct instanced_constants
	{
		mat4 view_projection; // world to the screen
	};
}

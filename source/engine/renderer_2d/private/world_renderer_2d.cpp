#include "renderer_2d/world_renderer_2d.h"

#include "renderer_2d/sprite_pass.h"

namespace buz
{
	//--------------------------------------------------------------
	world_renderer_2d::world_renderer_2d(const render_context& context, texture_filter sampler_filter)
		: renderer_2d(context, sampler_filter)
	{
	}

	//--------------------------------------------------------------
	void world_renderer_2d::set_camera(const mat4& view, const mat4& projection)
	{
		// Column-vector convention: the view applies first, then the
		// projection, so the projection comes first in the product.
		get_pass().set_view_projection(projection * view);
	}
}

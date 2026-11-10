#include "renderer_2d/world_renderer_2d.h"

#include "renderer_2d/sprite_pass.h"

namespace buz
{
	//--------------------------------------------------------------
	world_renderer_2d::world_renderer_2d(sprite_pass& pass)
		: renderer_2d(pass)
		, m_pass(pass)
	{
	}

	//--------------------------------------------------------------
	void world_renderer_2d::set_camera(const mat4& view, const mat4& projection)
	{
		// Column-vector convention: the view applies first, then the
		// projection, so the projection comes first in the product.
		m_pass.set_view_projection(projection * view);
	}
}

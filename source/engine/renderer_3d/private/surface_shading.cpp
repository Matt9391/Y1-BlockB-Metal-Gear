#include "renderer_3d/surface_shading.h"

#include "renderer_3d.h"

namespace buz
{
	//--------------------------------------------------------------
	void set_surface_shading(world_renderer_3d& world, const surface_shading& shading)
	{
		world.get_renderer().set_surface_shading(shading);
	}
}

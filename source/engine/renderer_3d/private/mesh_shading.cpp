#include "renderer_3d/mesh_shading.h"

#include "renderer_3d.h"

namespace buz
{
	//--------------------------------------------------------------
	void set_mesh_shading(world_renderer_3d& world, const mesh_shading& shading)
	{
		world.get_renderer().set_mesh_shading(shading);
	}
}

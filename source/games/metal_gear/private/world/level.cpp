#include "level.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/world_renderer_3d.h"
#include "renderer_3d/debug_draw.h"
#include "core/math.h"

namespace{
	const std::string_view k_collision_node_name = "col_"; 
}


level::level(const textured_model& map) :
    m_map(map)
{

    for (const auto& node : m_map.model.nodes)
	{
		buz::mat4 move = glm::translate(glm::mat4(1.0f), node.local_transform.translation);
		buz::mat4 turn = glm::mat4_cast(node.local_transform.rotation);
		buz::mat4 grow = glm::scale(glm::mat4(1.0f), node.local_transform.scale);
		buz::mat4 local_m4 = move * turn * grow;
		m_local_m4s.push_back(local_m4);
		
		//DONE change "col_" with some k costant so you dont have 2 failure point in the code (here and draw)
		if(node.name.starts_with(k_collision_node_name)){
			buz::vec3 centre = node.shape->get_bounds().center() + node.local_transform.translation;
			buz::vec3 size = node.shape->get_bounds().size() * node.local_transform.scale;

			m_solid_boxes.emplace_back(collider::box{centre, size});
		}	
				
		
	}
}

const std::vector<collider::box>& level::get_solid_boxes() const
{
	return m_solid_boxes;
}

void level::draw_3d(buz::world_renderer_3d& world_3d) const
{
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };

	for (size_t i = 0; i < m_map.model.nodes.size(); i++)
	{
		auto& node = m_map.model.nodes.at(i);
		auto& texture = m_map.textures[i];
		world_3d.draw(*node.shape, node.look, *texture, m_local_m4s.at(i)); 

		if(node.name.starts_with(k_collision_node_name)){
			buz::debug_draw::box(world_3d,
								 buz::aabb_from_center_size(m_solid_boxes.at(i).centre,
															m_solid_boxes.at(i).size), 
								 collider_color);
		}
	}

}

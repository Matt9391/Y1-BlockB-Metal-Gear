#include "level.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/world_renderer_3d.h"
#include "renderer_3d/debug_draw.h"
#include "core/math.h"

namespace
{
	const std::string_view k_collision_node_name = "col_";
}

level::level(const textured_model& map)
	: m_map(map)
	, m_collision_world(create_boxes(m_map.model))
{
	for (const auto& node : m_map.model.nodes)
	{
		buz::mat4 move = glm::translate(glm::mat4(1.0f), node.local_transform.translation);
		buz::mat4 turn = glm::mat4_cast(node.local_transform.rotation);
		buz::mat4 grow = glm::scale(glm::mat4(1.0f), node.local_transform.scale);
		buz::mat4 local_m4 = move * turn * grow;
		m_local_m4s.push_back(local_m4);
	}
}

const collision_world& level::get_collision_world() const{
	return m_collision_world;
}


void level::draw_3d(buz::world_renderer_3d& world_3d) const
{
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };

	for (size_t i = 0; i < m_map.model.nodes.size(); i++)
	{
		auto& node = m_map.model.nodes.at(i);
		auto& texture = m_map.textures[i];
		if(texture){
			world_3d.draw(*node.shape, node.look, *texture, m_local_m4s.at(i));
		}else{
			world_3d.draw(*node.shape, node.look, m_local_m4s.at(i));
		}
	}

	const std::vector<collider::box>& boxes = m_collision_world.get_boxes();

	for (size_t i = 0; i < boxes.size(); i++)
	{
		buz::debug_draw::box(world_3d,
							buz::aabb_from_center_size(boxes[i].centre,
							boxes[i].size),
							collider_color);
	}
}



std::vector<collider::box> level::create_boxes(const buz::model& map_model)
{	
	std::vector<collider::box> boxes;
	for (const auto& node : map_model.nodes)
	{
		// DONE change "col_" with some k costant so you dont have 2 failure point in the code (here and draw)
		if (node.name.starts_with(k_collision_node_name))
		{
			buz::vec3 centre = node.shape->get_bounds().center() + node.local_transform.translation;
			buz::vec3 size = node.shape->get_bounds().size() * node.local_transform.scale;

			boxes.emplace_back(collider::box{ centre, size });
		}
	}

	return boxes;
}

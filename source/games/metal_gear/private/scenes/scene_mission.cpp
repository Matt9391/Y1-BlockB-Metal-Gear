#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/window.h"
#include "core/log.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"
#include "collision/collision.h"
#include "characters/control.h"


std::vector<collider::box> boxes;
scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_)
	: scene(this_scene, next_scene, asset_cache_)
	, m_camera(60.f, { 0.f, 8.f, 10.f })
	, m_snake(get_asset_cache().get_model("models/soldier.glb"), { 0.f, 0.f, 9.f })
	, m_dock(get_asset_cache().get_model("models/dock_v6.glb"))
{
	// m_snake.get_character().set_pose(m_snake.get_character().get_model().find_clip("walk")->frames[4]);

	for (const auto& node : m_dock.nodes)
	{
		// buz::mat4 move = glm::translate(glm::mat4(1.0f), node.local_pose.translation);
		// buz::mat4 turn = glm::mat4_cast(node.local_pose.rotation);
		// buz::mat4 grow = glm::scale(glm::mat4(1.0f), node.local_pose.scale);
		// buz::mat4 local_m4 = move * turn * grow;
		if(node.name.starts_with("col_")){
			boxes.emplace_back(collider::box{node.shape->get_bounds().center() + node.local_pose.translation, node.shape->get_bounds().size() * node.local_pose.scale});
		}
	}
	// boxes.emplace_back(collider::box{b_centre, b_size});
}

void scene_mission::on_update(buz::engine& engine, float delta_seconds)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }
	if (input.was_key_pressed(buz::key::b)) { collision::collision_disabled = !collision::collision_disabled; buz::log::info("coll {}", collision::collision_disabled); }

	const buz::vec3 player_direction = player_control::compute_direction(input, m_camera.get_forward(), m_camera.get_right());
	m_snake.on_update(delta_seconds, player_direction, boxes);

	m_camera.set_target(m_snake.get_character().get_pos());
}


void scene_mission::on_render_world(buz::engine& engine, buz::world_renderer_3d& world_3d)
{
	world_3d.set_camera(m_camera.get_view_matrix(), m_camera.get_projx_matrix(engine.get_window().get_size()));
	// just for testing
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	// world_3d.draw_box({ 0.0f, 1.0f, 2.f }, { 5.0f, 1.0f, 5.f }, k_concrete);
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };


	for (const auto& node : m_dock.nodes)
	{
		buz::mat4 move = glm::translate(glm::mat4(1.0f), node.local_pose.translation);
		buz::mat4 turn = glm::mat4_cast(node.local_pose.rotation);
		buz::mat4 grow = glm::scale(glm::mat4(1.0f), node.local_pose.scale);
		buz::mat4 local_m4 = move * turn * grow;
		world_3d.draw(*node.shape, node.look, local_m4); 
		if(node.name.starts_with("col_")){
			buz::debug_draw::box(world_3d,buz::aabb_from_center_size(node.shape->get_bounds().center() + node.local_pose.translation, node.shape->get_bounds().size() * node.local_pose.scale), collider_color);
		}
	}


	

	m_snake.get_character().draw_3d(world_3d);
}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}

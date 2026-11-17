#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/window.h"
#include "core/log.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"

std::vector<buz::mat4> mats4;

scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_)
	: scene(this_scene, next_scene, asset_cache_)
	, m_camera(60.f, { 0.f, 5.f, 10.f })
	, m_snake(get_asset_cache().get_model("models/soldier.glb"), { 0.f, 0.f, 2.f })
{
	// m_c.set_pose(m_c.get_model().find_clip("walk")->frames[4]);
}
buz::vec3 root{ 0, 0, 2 };
buz::vec3 target{ 0.f, 0.f, 0.f };

void scene_mission::on_update(buz::engine& engine, float)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }

	m_snake.on_update(input, m_camera.get_forward(), m_camera.get_right());

	m_camera.set_target(m_snake.get_character().get_pos());
}

void scene_mission::on_render_world(buz::engine& engine, buz::world_renderer_3d& world_3d)
{
	world_3d.set_camera(m_camera.get_view_matrix(), m_camera.get_projx_matrix(engine.get_window().get_size()));
	// just for testing
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	// world_3d.draw_box({ 0.0f, 1.0f, 2.f }, { 5.0f, 1.0f, 5.f }, k_concrete);
	const buz::color collider_color = { 0.2f, 1.0f, 0.2f, 1.0f };

	buz::debug_draw::box(world_3d, buz::aabb_from_center_size({ 0.0f, 1.0f, 2.f }, { 5.0f, 1.0f, 5.f }), collider_color);

	m_snake.get_character().draw_3d(world_3d);
}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}

#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/window.h"
#include "core/log.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"
#include "collision/collision.h"
#include "characters/control.h"
#include "settings/game_settings.h"


scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_, game_settings& game_settings_)
	: scene(this_scene, next_scene, asset_cache_)
	, m_camera(20.f, { 0.f, 8.f, 10.f })
	, m_snake(get_asset_cache().get_model("models/soldier.glb"), game_settings_.get_snake())
	, m_level(get_asset_cache().get_model("models/dock_v6.glb"))
{
	// m_snake.get_character().set_pose(m_snake.get_character().get_model().find_clip("walk")->frames[4]);

	
	
	// boxes.emplace_back(collider::box{b_centre, b_size});
}

void scene_mission::on_update(buz::engine& engine, float delta_seconds)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }
	if (input.was_key_pressed(buz::key::b)) { collision::collision_disabled = !collision::collision_disabled; buz::log::info("coll {}", collision::collision_disabled); }

	const buz::vec3 player_direction = player_control::compute_direction(input, m_camera.get_forward(), m_camera.get_right());
	m_snake.on_update(delta_seconds, player_direction, m_level.get_solid_boxes());

	m_camera.set_target(m_snake.get_character().get_pos());
}


void scene_mission::on_render_world(buz::engine& engine, buz::world_renderer_3d& world_3d)
{
	world_3d.set_camera(m_camera.get_view_matrix(), m_camera.get_projx_matrix(engine.get_window().get_size()));

	m_level.draw_3d(world_3d);

	m_snake.get_character().draw_3d(world_3d);
}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}

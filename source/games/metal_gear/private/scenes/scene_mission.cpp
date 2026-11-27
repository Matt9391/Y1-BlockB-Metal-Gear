#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "engine/window.h"
#include "core/log.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"
#include "collision/collision_math.h"
#include "characters/control.h"
#include "settings/game_settings.h"

scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_, game_settings& game_settings_)
	: scene(this_scene, next_scene, asset_cache_)
	, m_camera(60.f, { 0.f, 8.f, 10.f })
	, m_snake(get_asset_cache().get_textured_model("sample_3d/models/soldier.glb"), game_settings_.get_snake())
	, m_level(get_asset_cache().get_textured_model("models/dock_v10.glb"))
	, m_view_cone(game_settings_.get_cone().view_distance_m, game_settings_.get_cone().cone_full_angle_deg)
{
	// TODO: change guard number to some variable or settings	
	for (int i = 0; i < 2; i++)
	{
		m_guard_units.emplace_back(guard_brain(m_level.get_path(i), game_settings_.get_guard().arrival_radius, game_settings_.get_guard().idle_time_s),
			character(get_asset_cache().get_textured_model("sample_3d/models/soldier.glb"),
				m_level.get_path(i).at(0),
				game_settings_.get_guard().turn_speed_deg_per_s,
				game_settings_.get_guard().walk_speed_m_per_s));
	}

}

void scene_mission::on_update(buz::engine& engine, float delta_seconds)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }
	if (input.was_key_pressed(buz::key::b))
	{
		collision_math::collision_disabled = !collision_math::collision_disabled;
		buz::log::info("coll {}", collision_math::collision_disabled);
	}

	const buz::vec3 player_direction = player_control::compute_direction(input, m_camera.get_forward(), m_camera.get_right());
	m_snake.on_update(delta_seconds, player_direction, m_level.get_collision_world());

	for (int i = 0; i < m_guard_units.size(); i++)
	{
		guard_unit& g = m_guard_units[i];
		g.brain.on_update(delta_seconds, g.body.get_pos(), false);
		const buz::vec3 guard_direction = g.brain.compute_direction(g.body.get_pos());
		g.body.on_update(delta_seconds, guard_direction, m_level.get_collision_world());
	}

	m_camera.set_target(m_snake.get_character().get_pos());
}

void scene_mission::on_render_world(buz::engine& engine, buz::world_renderer_3d& world_3d)
{
	world_3d.set_camera(m_camera.get_view_matrix(), m_camera.get_projx_matrix(engine.get_window().get_size()));

	m_level.draw_3d(world_3d);

	m_snake.get_character().draw_3d(world_3d);

	for (int i = 0; i < m_guard_units.size(); i++)
	{
		guard_unit& g = m_guard_units[i];
		g.body.draw_3d(world_3d);
		buz::debug_draw::cone(world_3d, g.body.get_centre(), g.body.get_forward(), m_view_cone.view_distance_m, m_view_cone.cone_full_angle_deg / 2.f, {0.f, 0.f, 1.f, 1.f});	
	}
}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}

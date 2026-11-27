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
#include "math/game_math.h"

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
		bool snake_detected = game_math::is_point_in_cone(g.body.get_centre(), g.body.get_forward(), m_view_cone, m_snake.get_character().get_centre());
		g.brain.on_update(delta_seconds, g.body.get_pos(), snake_detected);
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

		buz::color cone_color{0.f, 0.f, 1.f, 1.f};

		if(game_math::is_point_in_cone(g.body.get_centre(), g.body.get_forward(), m_view_cone, m_snake.get_character().get_centre())){
			cone_color = {0.5f, 0.f, 1.f, 1.f};
		}

		buz::debug_draw::cone(world_3d, 
							  g.body.get_centre(), 
							  g.body.get_forward(), 
							  m_view_cone.view_distance_m, 
							  buz::radians(m_view_cone.cone_full_angle_deg / 2.f), 
							  cone_color);	
		
	}
}

void scene_mission::on_render_screen(buz::engine& engine, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);

	buz::vec2 window_size = engine.get_window().get_size();
	float char_size = 48;
	for (int i = 0; i < m_guard_units.size(); i++)
	{
		guard_unit& g = m_guard_units[i];
		if(g.brain.get_state() == guard_state::spotted){
				//TODO: move them to a function draw_spotted_mark
				buz::vec3 point = g.body.get_centre();
				point.y += g.body.get_size().y / 1.5f;
				buz::vec4 spot_mark = m_camera.get_projx_matrix(window_size) * m_camera.get_view_matrix() * buz::vec4{point, 1.f};
				if(spot_mark.w <= 0){
					continue;
				}

				spot_mark /= spot_mark.w;
				
				buz::vec2 text_pos{game_math::remap(spot_mark.x, -1.f, 1.f, 0.f, window_size.x) - char_size / 2.f,
								  game_math::remap(spot_mark.y, -1.f, 1.f, window_size.y, 0.f)};

				screen.draw_text(get_font(), "!", text_pos, char_size, {1.f, 0.f, 0.f, 1.f});
		}
	}
}

#pragma once
#include "scene.h"
#include "camera/camera.h"
#include "characters/snake.h"
#include "characters/guard_unit.h"
#include "world/level.h"

class game_settings;

class scene_mission : public scene
{
public:
	scene_mission(scene_type this_scene, scene_type next_scene, asset_cache& asset_cache_, game_settings& game_settings_);
	~scene_mission() = default;
	//----------------------------------------------------------
	/// update logic
	void on_update(buz::engine& engine, float delta_seconds) override;

	/// draw object in the world
	void on_render_world(buz::engine& engine, buz::world_renderer_3d& world_3d) override;

	/// draws the HUD on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:
	camera m_camera;
	snake m_snake;
	level m_level;
	std::vector<guard_unit> m_guard_units;
};
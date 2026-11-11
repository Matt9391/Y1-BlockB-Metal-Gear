#pragma once
#include "scene.h"

class scene_mission : public scene
{
public:
	scene_mission(scene_type this_scene, scene_type next_scene, buz::world_renderer_3d& world_3d, const buz::font& font);
	~scene_mission() = default;
	//----------------------------------------------------------
	/// update logic
	void on_update(buz::engine& engine, float delta_seconds) override;

	/// draw object in the world
	void on_render_world(buz::engine& engine, buz::world_renderer_2d& world) override;

	/// draws the HUD on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:

};
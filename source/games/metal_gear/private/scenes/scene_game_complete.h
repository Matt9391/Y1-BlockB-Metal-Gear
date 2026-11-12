#pragma once
#include "scene.h"

class scene_game_complete : public scene
{
public:
	scene_game_complete(scene_type this_scene, scene_type next_scene, buz::world_renderer_3d& world_3d, asset_cache& asset_cache_);
	~scene_game_complete() = default;
	//----------------------------------------------------------
	/// update logic
	void on_update(buz::engine& engine, float delta_seconds) override;

	/// draw object in the world
	void on_render_world(buz::engine& engine, buz::world_renderer_2d& world) override;

	/// draws the HUD on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:

};
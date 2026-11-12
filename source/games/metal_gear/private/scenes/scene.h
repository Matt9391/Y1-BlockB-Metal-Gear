#pragma once

#include "renderer_3d/world_renderer_3d.h"
#include "renderer_2d/font.h"
#include "assets/asset_cache.h"

namespace buz
{
	class engine;
	class world_renderer_2d;
	class screen_renderer;
}

enum class scene_type
{
	menu,
	mission,
	game_over,
	game_complete
};

class scene
{
public:
	scene(scene_type this_scene, scene_type next_scene, buz::world_renderer_3d& world_3d, asset_cache& asset_cache_);;
	virtual ~scene() = default;
	//----------------------------------------------------------
	/// update logic
	virtual void on_update(buz::engine& engine, float delta_seconds);

	/// draw object in the world
	virtual void on_render_world(buz::engine& engine, buz::world_renderer_2d& world);

	/// draws the HUD on top.
	virtual void on_render_screen(buz::engine& engine, buz::screen_renderer& screen);


	// Getters
	scene_type get_scene_type() const { return m_scene_type; }
	scene_type get_next_scene_type() const { return m_next_scene_type; }

	bool get_invoke_next_scene() const { return m_invoke_next_scene; }

	buz::world_renderer_3d& get_world_3d() const { return m_world_3d; }
	asset_cache& get_asset_cache() const { return m_asset_cache; }
	const buz::font& get_font() const { return m_font; }

	// Setters
	void set_scene_type(scene_type type) { m_scene_type = type; }
	void set_next_scene_type(scene_type type) { m_next_scene_type = type; }
	
	void set_invoke_next_scene(bool value) { m_invoke_next_scene = value; }

private:
	scene_type m_scene_type;
	scene_type m_next_scene_type;

	bool m_invoke_next_scene;

	buz::world_renderer_3d& m_world_3d;
	asset_cache& m_asset_cache;
	const buz::font& m_font;

};
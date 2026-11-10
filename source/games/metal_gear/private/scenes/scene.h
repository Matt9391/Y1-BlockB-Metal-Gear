#pragma once

#include "renderer_3d/world_renderer_3d.h"
#include "renderer_2d/font.h"

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
	scene(scene_type this_scene, scene_type next_scene, buz::engine& engine, buz::world_renderer_3d& world_3d, const buz::font& font);
	virtual ~scene() = default;
	//----------------------------------------------------------
	/// update logic
	virtual void on_update(buz::engine& engine, float delta_seconds);

	/// draw object in the world
	virtual void on_render_world(buz::engine& engine, buz::world_renderer_2d& world);

	/// draws the HUD on top.
	virtual void on_render_screen(buz::engine& engine, buz::screen_renderer& screen);

private:
	scene_type m_scene_type;
	scene_type m_next_scene_type;

	bool m_invoke_next_scene;

	buz::world_renderer_3d& m_world_3d;
	const buz::font& m_font; // for the hud
};

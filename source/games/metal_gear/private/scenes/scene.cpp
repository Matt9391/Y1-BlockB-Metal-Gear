#include "scene.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "renderer_2d/screen_renderer.h"


scene::scene(scene_type this_scene, scene_type next_scene, buz::world_renderer_3d& world_3d, asset_cache& asset_cache_)
	: m_scene_type{ this_scene }
	, m_next_scene_type{ next_scene }
	, m_invoke_next_scene{ false }
	, m_world_3d{ world_3d }
	, m_asset_cache{ asset_cache_ }
	, m_font(m_asset_cache.get_font("fonts/font_8x8.png"))
{
}

void scene::on_update(buz::engine& engine, float)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { m_invoke_next_scene = true; }
	
}   

void scene::on_render_world(buz::engine&, buz::world_renderer_2d&)
{
	// just for testing
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	m_world_3d.draw_box({ 0.0f, 1.0f, 5.f }, { 0.0f, 1.0f, 5.f }, k_concrete);
}

void scene::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "base scene", { 16.0f, 16.0f }, 12.0f, text_color);
}

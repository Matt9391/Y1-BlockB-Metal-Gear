#include "scene_mission.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "renderer_2d/screen_renderer.h"

scene_mission::scene_mission(scene_type this_scene, scene_type next_scene, buz::world_renderer_3d& world_3d, const buz::font& font) :
    scene(this_scene,next_scene, world_3d, font)
{
}

void scene_mission::on_update(buz::engine& engine, float)
{
	const buz::input& input = engine.get_input();
	if (input.was_key_pressed(buz::key::space)) { set_invoke_next_scene(true); }
	
}   

void scene_mission::on_render_world(buz::engine&, buz::world_renderer_2d&)
{
	// just for testing
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	get_world_3d().draw_box({ 0.0f, 1.0f, 5.f }, { 0.0f, 1.0f, 5.f }, k_concrete);
}

void scene_mission::on_render_screen(buz::engine&, buz::screen_renderer& screen)
{
	const buz::color text_color = { 1.0f, 1.0f, 1.0f, 0.8f };
	screen.draw_text(get_font(), "mission scene scene", { 16.0f, 16.0f }, 12.0f, text_color);
}
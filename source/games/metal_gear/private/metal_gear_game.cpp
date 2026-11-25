#include "metal_gear_game.h"

#include "core/aabb.h"
#include "engine/engine.h"
#include "engine/input.h"
#include "asset_management/model_loading.h"
#include "asset_management/texture_loading.h"
#include "renderer_2d/screen_renderer.h"
#include "renderer_3d/debug_draw.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"
#include "engine/window.h"

#include <iterator>

namespace
{
	// Greybox colors: flat, so the shapes read clearly
	const buz::material k_concrete = { { 0.55f, 0.55f, 0.58f, 1.0f } };
	const buz::material k_wall = { { 0.42f, 0.44f, 0.48f, 1.0f } };
	const buz::material k_water = { { 0.10f, 0.22f, 0.32f, 1.0f } };
	const buz::material k_rust = { { 0.55f, 0.30f, 0.18f, 1.0f } };
	const buz::material k_buoy = { { 0.90f, 0.35f, 0.10f, 1.0f } };
	const buz::material k_fatigues = { { 0.36f, 0.40f, 0.30f, 1.0f } };

	// Unlit: a pickup glows the same color from every side
	const buz::material k_pickup = { { 0.30f, 1.00f, 0.50f, 1.0f }, {}, buz::lighting::no };
}

//--------------------------------------------------------------
metal_gear_game::metal_gear_game(buz::engine& engine)
	: buz::game_3d(engine)
	, m_asset_cache(engine)
	, m_game_settings("settings/game_settings.json")
	, m_scene_handler(scene_type::menu, m_asset_cache, m_game_settings)
{
}

//--------------------------------------------------------------
void metal_gear_game::on_update(buz::engine& engine, float dt_s)
{
	m_scene_handler.get_current_scene().on_update(engine, dt_s);
	if (m_scene_handler.has_pending_scene())
	{
		m_scene_handler.switch_scene();
	}
}

//--------------------------------------------------------------
void metal_gear_game::on_render_world(buz::engine& engine, buz::world_renderer_3d& world)
{
	m_scene_handler.get_current_scene().on_render_world(engine, world);
}

//--------------------------------------------------------------
void metal_gear_game::on_render_screen(buz::engine& engine, buz::screen_renderer& screen)
{
	m_scene_handler.get_current_scene().on_render_screen(engine, screen);
}

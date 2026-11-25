#pragma once

#include "core/math.h"
#include "game_framework/game_3d.h"
#include "renderer_2d/font.h"
#include "renderer_3d/material.h"
#include "renderer_3d/world_renderer_3d.h"
#include "scenes/scene_handler.h"
#include "assets/asset_cache.h"
#include "settings/game_settings.h"

#include <memory>
#include <vector>

namespace buz
{
	class mesh;
}

//--------------------------------------------------------------
/// @brief The Block B starting point: a greybox harbour built from the
/// built-in shapes, a crate loaded from a glTF file, and a soldier model
/// made of rigid parts, standing as he was modelled.
///
/// Things to build from here: a camera that follows the soldier, play his
/// animations, move him around, collision with the crates...
class metal_gear_game : public buz::game_3d
{
public:
	//----------------------------------------------------------
	/// @brief Loads the models and textures.
	explicit metal_gear_game(buz::engine& engine);

protected:
	//----------------------------------------------------------
	/// @brief Switches the debug views with the function keys.
	void on_update(buz::engine& engine, float dt_s) override;

	//----------------------------------------------------------
	/// @brief Draws the harbour, the crates and the soldier.
	void on_render_world(buz::engine& engine, buz::world_renderer_3d& world) override;

	//----------------------------------------------------------
	/// @brief Draws the controls on top.
	void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override;

private:

	asset_cache m_asset_cache;
	game_settings m_game_settings;
	scene_handler m_scene_handler;
};

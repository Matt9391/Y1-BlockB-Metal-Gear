/// @file
/// @brief Draws on top of the world, in window pixels.

#pragma once

#include "renderer_2d/renderer_2d.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Draws on top of the world in plain window pixels: score,
	/// lives, menus. It ignores the world's camera, so the hud stays put
	/// while the level scrolls underneath it.
	///
	/// Every game has one, owned by buz::game and handed to on_render_screen
	/// every frame, after on_render_world.
	class screen_renderer : public renderer_2d
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates the renderer and its pipeline on the engine's
		/// graphics card; buz::game makes one for your game.
		/// @param context The engine's render context
		/// (engine::get_render_context).
		/// @param sampler_filter How its textures are filtered when scaled
		/// on screen (engine_settings::sampler_filter).
		screen_renderer(const render_context& context, texture_filter sampler_filter);
	};
}

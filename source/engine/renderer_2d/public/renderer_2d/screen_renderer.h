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
	/// Handed to game::on_render_screen every frame, after on_render_world.
	class screen_renderer : public renderer_2d
	{
	public:
		//----------------------------------------------------------
		/// @brief Draws into the engine's screen pass; only the engine has
		/// one to pass in.
		explicit screen_renderer(sprite_pass& pass);
	};
}

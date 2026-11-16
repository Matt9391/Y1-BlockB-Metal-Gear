/// @file
/// @brief What every game shares: the main loop's calls and the screen renderer.

#pragma once

#include "renderer_2d/screen_renderer.h"

namespace buz
{
	class engine;

	//--------------------------------------------------------------
	/// @brief The base class of buz::game_2d and buz::game_3d: a game derives
	/// from one of those two, depending on whether its world is 2D or 3D.
	///
	/// Every game has two renderers. The world renderer draws the level and
	/// belongs to game_2d (a world_renderer_2d) or game_3d (a
	/// world_renderer_3d). The screen renderer draws the hud on top of it in
	/// window pixels and belongs to this class. A game reaches each renderer
	/// only in the function that gets it: on_render_world and
	/// on_render_screen.
	///
	/// The engine owns the main loop and calls update, render_world and
	/// render_screen every frame; they call the on_ functions a game
	/// overrides.
	class game
	{
	public:
		virtual ~game();

		game(const game&) = delete;
		game& operator=(const game&) = delete;

		//----------------------------------------------------------
		/// @brief Called by the engine every frame before drawing.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		void update(engine& engine, float delta_seconds);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame to draw the game world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		void render_world(engine& engine);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame after render_world, to draw
		/// on top of the world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		void render_screen(engine& engine);

	protected:
		//----------------------------------------------------------
		/// @brief Creates the screen renderer.
		/// @param engine Engine instance, whose graphics card the renderer draws with.
		explicit game(engine& engine);

		//----------------------------------------------------------
		/// @brief Override to move the game on: input, movement, collisions.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		virtual void on_update(engine& engine, float delta_seconds);

		//----------------------------------------------------------
		/// @brief Override to draw on top of the world in window pixels:
		/// score, lives, menus.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param screen Renderer that draws in window pixels.
		virtual void on_render_screen(engine& engine, screen_renderer& screen);

		//----------------------------------------------------------
		/// @brief Draws the world. game_2d and game_3d implement it by passing
		/// their world renderer to on_render_world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		virtual void draw_world(engine& engine) = 0;

	private:
		screen_renderer m_screen_renderer; // draws the hud, handed to on_render_screen
	};
}

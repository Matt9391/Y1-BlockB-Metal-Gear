/// @file
/// @brief The base class of a game with a 2D world.

#pragma once

#include "engine/game.h"
#include "renderer_2d/world_renderer_2d.h"

namespace buz
{
	//--------------------------------------------------------------
	/// @brief The base class of a game with a 2D world: derive your game from
	/// it.
	///
	/// It owns the game's world renderer, a buz::world_renderer_2d, and hands
	/// it to on_render_world every frame; the screen renderer comes from
	/// buz::game. Load what the game needs in its constructor, which gets the
	/// engine, and let the destructor (or the members' own) clean up.
	/// Override the functions you need:
	/// @code{.cpp}
	/// class my_game : public buz::game_2d
	/// {
	/// public:
	///     explicit my_game(buz::engine& engine);                                             // load textures here
	///
	/// protected:
	///     void on_update(buz::engine& engine, float delta_seconds) override;                 // game logic here
	///     void on_render_world(buz::engine& engine, buz::world_renderer_2d& world) override;    // draw the level here
	///     void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override; // draw the hud here
	/// };
	///
	/// my_game::my_game(buz::engine& engine)
	///     : buz::game_2d(engine)
	/// {
	/// }
	/// @endcode
	class game_2d : public game
	{
	protected:
		//----------------------------------------------------------
		/// @brief Creates the screen renderer and the 2D world renderer.
		/// @param engine Engine instance, whose graphics card the renderers draw with.
		explicit game_2d(engine& engine);

		//----------------------------------------------------------
		/// @brief Override to draw the game world: everything that moves when
		/// the camera moves.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param world Renderer that draws in world positions, through the camera.
		virtual void on_render_world(engine& engine, world_renderer_2d& world);

	private:
		void draw_world(engine& engine) final;

		world_renderer_2d m_world_renderer_2d; // draws the level, handed to on_render_world
	};
}

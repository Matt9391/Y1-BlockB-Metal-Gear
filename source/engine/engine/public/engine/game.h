/// @file
/// @brief The base class every game derives from.

#pragma once

namespace buz
{
	class engine;
	class screen_renderer;
	class world_renderer_2d;

	//--------------------------------------------------------------
	/// @brief Base class every game derives from.
	///
	/// The engine owns the main loop and calls update, render_world and
	/// render_screen every frame; they call the on_ functions a game
	/// overrides. Load what the game needs in its constructor, which gets the
	/// engine, and let the destructor (or the members' own) clean up.
	/// Override the functions you need:
	/// @code{.cpp}
	/// class my_game : public buz::game
	/// {
	/// public:
	///     explicit my_game(buz::engine& engine);                                             // load textures here
	///
	/// protected:
	///     void on_update(buz::engine& engine, float delta_seconds) override;                 // game logic here
	///     void on_render_world(buz::engine& engine, buz::world_renderer_2d& world) override;    // draw the level here
	///     void on_render_screen(buz::engine& engine, buz::screen_renderer& screen) override; // draw the hud here
	/// };
	/// @endcode
	class game
	{
	public:
		virtual ~game();

		//----------------------------------------------------------
		/// @brief Called by the engine every frame before drawing.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		void update(engine& engine, float delta_seconds);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame to draw the game world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param world Renderer that draws in world positions, through the camera.
		void render_world(engine& engine, world_renderer_2d& world);

		//----------------------------------------------------------
		/// @brief Called by the engine every frame after render_world, to draw
		/// on top of the world.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param screen Renderer that draws in window pixels.
		void render_screen(engine& engine, screen_renderer& screen);

	protected:
		//----------------------------------------------------------
		/// @brief Override to move the game on: input, movement, collisions.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param delta_seconds Time the previous frame took, in seconds.
		virtual void on_update(engine& engine, float delta_seconds);

		//----------------------------------------------------------
		/// @brief Override to draw the game world: everything that moves when
		/// the camera moves.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param world Renderer that draws in world positions, through the camera.
		virtual void on_render_world(engine& engine, world_renderer_2d& world);

		//----------------------------------------------------------
		/// @brief Override to draw on top of the world in window pixels:
		/// score, lives, menus.
		/// @param engine Engine instance, use it to read input and reach other systems.
		/// @param screen Renderer that draws in window pixels.
		virtual void on_render_screen(engine& engine, screen_renderer& screen);
	};
}

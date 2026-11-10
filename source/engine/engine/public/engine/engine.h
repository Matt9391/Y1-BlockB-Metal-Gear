/// @file
/// @brief The engine: owns every system and runs the main loop.

#pragma once

#include "engine/engine_settings.h"

#include <memory>

namespace buz
{
	class audio;
	class game;
	class input;
	class screenshot_saver;
	class window;

	struct engine_systems;
	struct render_context;

	//--------------------------------------------------------------
	/// @brief Owns all engine systems and runs the main loop.
	///
	/// The engine comes first, the game second: the game needs the engine to
	/// load its textures, so it takes it in its constructor. C++ destroys
	/// local variables in reverse order, so the game, and every texture and
	/// mesh it holds, is gone before the engine shuts the graphics card down.
	/// @code{.cpp}
	/// int main(int argc, char** argv)
	/// {
	///     buz::engine engine(argc, argv, { .window_title = "my game" });
	///     my_game game(engine);
	///     return engine.run(game);
	/// }
	/// @endcode
	class engine
	{
		//----------------------------------------------------------
		// Lifetime
	public:
		//----------------------------------------------------------
		/// @brief Opens the window and starts every system. When that is
		/// impossible (no graphics card that can run the game, say) the
		/// engine stops with an error message: nothing can run without them.
		///
		/// The settings on the command line (see engine_settings) win over
		/// the game's, so they can change without rebuilding.
		/// @param argc The argument count main got.
		/// @param argv The arguments main got.
		/// @param game_settings The game's startup settings.
		engine(int argc, char** argv, const engine_settings& game_settings);
		~engine();

		engine(const engine&) = delete;
		engine& operator=(const engine&) = delete;

		//----------------------------------------------------------
		/// @brief Runs the game until the window closes or quit() is called.
		/// @param active_game The game to run.
		/// @return Exit code for main (0 on success).
		int run(game& active_game);

		//----------------------------------------------------------
		// The systems a game uses
	public:
		//----------------------------------------------------------
		/// @brief Returns the keyboard and mouse state of this frame.
		const input& get_input() const;

		//----------------------------------------------------------
		/// @brief Returns the sound effect and music player.
		audio& get_audio();

		//----------------------------------------------------------
		/// @brief Returns the window the game draws into.
		const window& get_window() const;

		//----------------------------------------------------------
		/// @brief Returns what saves frames as png files.
		screenshot_saver& get_screenshots();

		//----------------------------------------------------------
		// While the game runs
	public:
		//----------------------------------------------------------
		/// @brief Closes the game at the end of this frame.
		void quit();

		//----------------------------------------------------------
		// For engine modules
	public:
		//----------------------------------------------------------
		/// @brief Returns what engine modules (the 3D renderer, model
		/// loading) create gpu resources with. A game cannot use it: the
		/// type lives in the engine's internal headers.
		const render_context& get_render_context() const;

	private:
		// Every system, in the order they start; defined in engine.cpp, so
		// including this header does not pull them all into a game.
		std::unique_ptr<engine_systems> m_systems;
	};
}

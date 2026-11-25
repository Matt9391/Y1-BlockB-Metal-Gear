/// @file
/// @brief The engine: owns every system and runs the main loop.

#pragma once

#include "engine/engine_settings.h"

#include <memory>

namespace buz
{
	class audio;
	class input;
	class playable;
	class screenshot_saver;
	class window;

	struct engine_systems;
	struct render_context;

	//--------------------------------------------------------------
	/// @brief Owns all engine systems and runs the main loop.
	///
	/// Create the engine before the game: the game needs the engine to load
	/// its textures, so it takes the engine in its constructor. C++ destroys
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
		/// @brief Opens the window and starts every system. When a system
		/// cannot start (for example, no graphics card can run the game), the
		/// engine stops with an error message, because the game cannot run
		/// without that system.
		///
		/// The settings on the command line (see engine_settings) override
		/// the game's, so they can be changed without rebuilding.
		/// @param argc The argument count main got.
		/// @param argv The arguments main got.
		/// @param game_settings The game's startup settings.
		engine(int argc, char** argv, const engine_settings& game_settings);
		~engine();

		engine(const engine&) = delete;
		engine& operator=(const engine&) = delete;

		//----------------------------------------------------------
		/// @brief Runs the game until the window closes or quit() is called.
		/// @param active_game The game to run: a buz::game_2d or buz::game_3d,
		/// or anything else that implements buz::playable.
		/// @return Exit code for main (0 on success).
		int run(playable& active_game);

		//----------------------------------------------------------
		// The systems a game uses
	public:
		//----------------------------------------------------------
		/// @brief Returns the keyboard and mouse state of this frame, and
		/// the cursor lock.
		input& get_input();

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
		/// @brief Returns the settings the engine runs with: the game's, with
		/// the command line's on top.
		const engine_settings& get_settings() const;

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
		/// @brief Returns what engine modules (the game framework's
		/// renderers, asset loading) create gpu resources with. A game cannot
		/// use it: render_context is only declared here, and its definition
		/// is in render_core's internal headers.
		const render_context& get_render_context() const;

	private:
		// Every system, in the order they start; defined in engine.cpp, so
		// including this header does not pull them all into a game.
		std::unique_ptr<engine_systems> m_systems;
	};
}

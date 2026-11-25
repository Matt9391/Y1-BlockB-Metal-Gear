#pragma once

// Where the engine's files are on this machine. For engine modules only: a
// game cannot include this header, on purpose.
//
// Every public function that reads a file (load_texture, load_model,
// play_sound, load_json, ...) takes a path relative to the content folder,
// with / between folders, and what a game writes (screenshots) goes to its
// own folder under saved/. Only the engine turns those into files on this
// machine, with the functions below. That keeps three things true:
// - Games never depend on where the executable or the content folder is, so
//   the same code runs from Visual Studio, from a package and in the browser.
// - A platform that stores its files differently (a console, say) needs a
//   change in this one place, not in every game.
// - Game headers don't pull in <filesystem> or real file paths.

#include "core/filesystem.h"
#include <string_view>

namespace buz
{
	namespace paths
	{
		//----------------------------------------------------------
		/// @brief Returns the directory the running executable lives in.
		///
		/// All engine content (compiled shaders, textures, the pipeline
		/// cache, ...) is loaded relative to this directory, so the game
		/// works no matter which working directory it is started from.
		fs::path get_executable_directory();

		//----------------------------------------------------------
		/// @brief Returns the directory game content is loaded from.
		///
		/// This is @c get_executable_directory() / "content", the folder the
		/// build copies textures, fonts and shaders into next to the
		/// executable.
		fs::path get_content_directory();

		//----------------------------------------------------------
		/// @brief Turns a path the api takes, relative to the content folder
		/// with / between folders ("my_game/music/theme.mp3"), into the file on this
		/// machine, inside get_content_directory().
		fs::path get_content_path(std::string_view content_relative_path);

		//----------------------------------------------------------
		/// @brief Returns the directory the running game writes its files
		/// (screenshots, save files) to: saved/<game> next to the
		/// executable, e.g. build/Debug/saved/sample_2d.
		///
		/// Every game builds into the same folder, so each one gets a
		/// folder of its own and does not read another game's saves.
		fs::path get_output_directory();

		//----------------------------------------------------------
		/// @brief Turns a path of a file the engine writes (a screenshot, a
		/// save file) into the file on this machine: a relative path ends up
		/// in get_output_directory(), an absolute one stays as it is.
		fs::path get_output_path(std::string_view path);
	}
}

#pragma once

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
		/// with / between folders ("music/theme.mp3"), into the file on this
		/// machine. The one place that knows where content lives.
		fs::path get_content_path(std::string_view content_relative_path);

		//----------------------------------------------------------
		/// @brief Returns the directory the running game writes its files
		/// (screenshots, save files) to: saved/<game> next to the
		/// executable, e.g. build/Debug/saved/sample_2d.
		///
		/// Every game builds into the same folder, so each one gets a
		/// folder of its own: a game never finds another game's saves.
		fs::path get_output_directory();

		//----------------------------------------------------------
		/// @brief Turns a path of a file the engine writes (a screenshot, a
		/// save file) into the file on this machine: a relative path ends up
		/// in get_output_directory(), an absolute one stays as it is.
		fs::path get_output_path(std::string_view path);
	}
}

/// @file
/// @brief The file system: its path type and the operations buz needs.

#pragma once

#include <version>

// Each platform picks its path type here, in one place. Use buz::fs::path
// instead of naming the platform's type directly, so a new platform only
// adds a branch to this file.
#if defined(__cpp_lib_filesystem)
	#include <filesystem>

namespace buz
{
	namespace fs
	{
		//----------------------------------------------------------
		/// @brief The path type of the file system buz runs on.
		using path = std::filesystem::path;
	}
}

#else
	#error "Unsupported platform: no std::filesystem. Add this platform's path type to core/filesystem.h."
#endif

namespace buz
{
	namespace fs
	{
		//----------------------------------------------------------
		/// @brief Returns whether a file or directory exists at the path.
		bool exists(const path& file);

		//----------------------------------------------------------
		/// @brief Creates the directory and any parents that are missing.
		/// @return True when the directory exists afterwards.
		bool create_directories(const path& directory);

		//----------------------------------------------------------
		/// @brief Deletes a file, or an empty directory.
		/// @return True when nothing exists at the path afterwards.
		bool remove(const path& file);
	}
}

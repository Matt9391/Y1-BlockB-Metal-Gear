// The file system on every platform with std::filesystem (all of buz's
// desktop platforms). A platform without it adds its own folder next to this
// one, and core's CMakeLists.txt excludes this folder there.
#include "core/filesystem.h"

#include "core/log.h"
#include "core/paths.h"

#include <algorithm>
#include <system_error>

namespace buz
{
	namespace fs
	{
		//----------------------------------------------------------
		bool exists(const path& file)
		{
			std::error_code error;
			return std::filesystem::exists(file, error);
		}

		//----------------------------------------------------------
		bool create_directories(const path& directory)
		{
			std::error_code error;
			std::filesystem::create_directories(directory, error);
			return !error;
		}

		//----------------------------------------------------------
		bool remove(const path& file)
		{
			std::error_code error;
			std::filesystem::remove(file, error);
			return !error;
		}

		//----------------------------------------------------------
		std::vector<std::string> list_content_files(std::string_view content_relative_directory)
		{
			const path content = paths::get_content_directory();
			const path directory = paths::get_content_path(content_relative_directory);

			std::error_code error;
			std::filesystem::recursive_directory_iterator entry(directory, error);
			if (error)
			{
				log::error("Could not list content folder \"{}\": {}", content_relative_directory, error.message());
				return {};
			}

			// A range-for advances with operator++, which throws when a
			// folder cannot be read, so the loop steps with increment(error)
			std::vector<std::string> files;
			for (; entry != std::filesystem::recursive_directory_iterator(); entry.increment(error))
			{
				// The api uses utf-8 paths with / between folders on every platform
				std::error_code type_error;
				if (entry->is_regular_file(type_error))
				{
					const std::u8string relative = entry->path().lexically_relative(content).generic_u8string();
					files.emplace_back(relative.begin(), relative.end());
				}
			}

			// A failed step ends the walk early (the iterator turns into the
			// end), so whatever was found so far is returned
			if (error)
			{
				log::error("Could not list all of content folder \"{}\": {}", content_relative_directory, error.message());
			}

			// The order of directory entries depends on the file system, so
			// the list is sorted to load the files in the same order every run
			std::sort(files.begin(), files.end());
			return files;
		}
	}
}

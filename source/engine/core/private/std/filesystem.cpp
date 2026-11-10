// The file system on every platform with std::filesystem (all of buz's
// desktop platforms). A platform without it adds its own folder next to this
// one, and core's CMakeLists.txt excludes this folder there.
#include "core/filesystem.h"

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
	}
}

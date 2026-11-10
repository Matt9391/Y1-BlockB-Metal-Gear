#include "core/paths.h"

#include <string>

#if defined(_WIN32)
	#define WIN32_LEAN_AND_MEAN
	#include <windows.h>
#endif

namespace
{
	//--------------------------------------------------------------
	/// @brief Turns the api's text into a path. The text is utf-8, which
	/// std::filesystem only assumes when told (char8_t); its / reads as a
	/// folder separator on Windows too.
	buz::fs::path to_path(std::string_view utf8_text)
	{
		return std::u8string(utf8_text.begin(), utf8_text.end());
	}

#if !defined(__EMSCRIPTEN__)
	//--------------------------------------------------------------
	/// @brief The running executable's file, e.g. build/Debug/sample_2d.exe.
	buz::fs::path get_executable_file()
	{
	#if defined(_WIN32)
		wchar_t executable_path[MAX_PATH];
		GetModuleFileNameW(nullptr, executable_path, MAX_PATH);

		return executable_path;
	#else
		// Linux: /proc/self/exe is a link to the running executable
		return std::filesystem::read_symlink("/proc/self/exe");
	#endif
	}
#endif
}

namespace buz
{
	namespace paths
	{
		//----------------------------------------------------------
		fs::path get_executable_directory()
		{
#if defined(__EMSCRIPTEN__)
			// A browser has no executable file; the web package puts the
			// content in its virtual file system at /content
			return "/";
#else
			return get_executable_file().parent_path();
#endif
		}

		//----------------------------------------------------------
		fs::path get_output_directory()
		{
#if defined(__EMSCRIPTEN__)
			// A web page holds one game, so it needs no folder of its own
			return "/saved";
#else
			return get_executable_directory() / "saved" / get_executable_file().stem();
#endif
		}

		//----------------------------------------------------------
		fs::path get_content_directory()
		{
			return get_executable_directory() / "content";
		}

		//----------------------------------------------------------
		fs::path get_content_path(std::string_view content_relative_path)
		{
			return get_content_directory() / to_path(content_relative_path);
		}

		//----------------------------------------------------------
		fs::path get_output_path(std::string_view path)
		{
			const fs::path output = to_path(path);
			return output.is_absolute() ? output : get_output_directory() / output;
		}
	}
}

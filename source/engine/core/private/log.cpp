#include "core/log.h"

#if defined(BUZ_LOG_ENABLED)

	#include "core/paths.h"

	#if defined(_WIN32)
		#define WIN32_LEAN_AND_MEAN
		#include <windows.h>
	#endif

	#include <cstdio>
	#include <fstream>
	#include <string>

namespace
{
	//--------------------------------------------------------------
	/// @brief Enables colored output on the windows console.
	void enable_console_colors()
	{
	#if defined(_WIN32)
		// Linux terminals understand the color codes already; the windows
		// console has to be asked.
		const HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);

		DWORD console_mode = 0;
		if (GetConsoleMode(console_handle, &console_mode))
		{
			SetConsoleMode(console_handle, console_mode | ENABLE_VIRTUAL_TERMINAL_PROCESSING);
		}
	#endif
	}

	//--------------------------------------------------------------
	/// @brief Where log lines go: in color to the console (and the Visual
	/// Studio output window), and without color to buz.log next to the
	/// executable.
	class log_output
	{
	public:
		//----------------------------------------------------------
		/// @brief Opens the log file and turns on console colors.
		log_output();

		//----------------------------------------------------------
		/// @brief Writes one line everywhere.
		void write(const char* name, const char* color, std::string_view text);

	private:
		std::ofstream m_file;
	};

	//--------------------------------------------------------------
	log_output::log_output()
		: m_file(buz::paths::get_executable_directory() / "buz.log", std::ios::trunc)
	{
		enable_console_colors();
	}

	//--------------------------------------------------------------
	void log_output::write(const char* name, const char* color, std::string_view text)
	{
		const int text_length = static_cast<int>(text.size());
		const char* text_start = text.data();
		std::printf("%s[%s]%s %.*s\n", color, name, buz::log::k_reset, text_length, text_start);

	#if defined(_WIN32)
		const std::string line = std::format("[{}] {}\n", name, text);
		OutputDebugStringA(line.c_str());
	#endif

		if (m_file.is_open())
		{
			m_file << '[' << name << "] " << text << '\n';
			m_file.flush();
		}
	}

	//--------------------------------------------------------------
	/// @brief Returns the log output, created on the first log line.
	log_output& get_output()
	{
		static log_output output;
		return output;
	}
}

namespace buz
{
	namespace log
	{
		namespace internal
		{
			//----------------------------------------------------------
			void trace(const char* name, const char* color, std::string_view text)
			{
				get_output().write(name, color, text);
			}
		}
	}
}

#else

namespace buz
{
	namespace log
	{
		namespace internal
		{
			//----------------------------------------------------------
			void trace(const char*, const char*, std::string_view)
			{
				// A shipping build logs nothing: the log functions never call
				// this, it only exists so the program links.
			}
		}
	}
}

#endif

/// @file
/// @brief Logging info, warnings and errors to the console and a log file.

#pragma once

#include <format>
#include <string_view>
#include <utility>

// Logging is compiled in for Debug and Development builds. In Release, the
// shipping build (NDEBUG without BUZ_DEVELOPMENT), every log call collapses
// to nothing, so no formatting cost is paid and no log file is created. The format string is still validated at compile time in both
// builds: std::format_string checks the placeholders against the argument
// types while compiling, so a typo like log::info("{} {}", one_value) is a
// compile error, not a runtime surprise. If you ever need a format string
// that is only known at runtime, that is what std::vformat is for.
#if !defined(NDEBUG) || defined(BUZ_DEVELOPMENT)
	#define BUZ_LOG_ENABLED
#endif

namespace buz
{
	namespace log
	{
		/// @brief Console color codes: a log line starts in its color and ends with k_reset.
		constexpr auto k_green = "\033[32m";
		constexpr auto k_magenta = "\033[35m";
		constexpr auto k_red = "\033[31m";
		constexpr auto k_reset = "\033[0m";

		namespace internal
		{
			//------------------------------------------------------------------
			/// @brief Fans a formatted line out to every registered sink.
			void trace(const char* name, const char* color, std::string_view text);
		}

		//----------------------------------------------------------
		/// @brief Logs an informational message (std::format "{}" style formatting).
		/// @param format Format string with "{}" placeholders, checked at compile time.
		/// @param args Values inserted into the placeholders.
		template<typename... Args>
		void info(std::format_string<Args...> format, Args&&... args)
		{
#if defined(BUZ_LOG_ENABLED)
			internal::trace("info", k_green, std::format(format, std::forward<Args>(args)...));
#else
			(void)format;
			((void)args, ...);
#endif
		}

		//----------------------------------------------------------
		/// @brief Logs a warning message (std::format "{}" style formatting).
		/// @param format Format string with "{}" placeholders, checked at compile time.
		/// @param args Values inserted into the placeholders.
		template<typename... Args>
		void warn(std::format_string<Args...> format, Args&&... args)
		{
#if defined(BUZ_LOG_ENABLED)
			internal::trace("warn", k_magenta, std::format(format, std::forward<Args>(args)...));
#else
			(void)format;
			((void)args, ...);
#endif
		}

		//----------------------------------------------------------
		/// @brief Logs an error message (std::format "{}" style formatting).
		/// @param format Format string with "{}" placeholders, checked at compile time.
		/// @param args Values inserted into the placeholders.
		template<typename... Args>
		void error(std::format_string<Args...> format, Args&&... args)
		{
#if defined(BUZ_LOG_ENABLED)
			internal::trace("error", k_red, std::format(format, std::forward<Args>(args)...));
#else
			(void)format;
			((void)args, ...);
#endif
		}
	}
}

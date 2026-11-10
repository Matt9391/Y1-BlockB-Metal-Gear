/// @file
/// @brief Assertions that stop in the debugger, and fatal_error for what the engine cannot recover from.

#pragma once

#include <format>
#include <source_location>
#include <string_view>
#include <utility>

// Asserts are compiled in for debug builds only, exactly like logging. In
// release (NDEBUG) a BUZ_ASSERT disappears completely: the condition is not
// even evaluated, so never put code with side effects inside one.
#if !defined(NDEBUG)
	#define BUZ_ASSERT_ENABLED
#endif

// Stops the program in the debugger, on every compiler.
#if defined(_MSC_VER)
	#define BUZ_DEBUG_BREAK() __debugbreak()
#else
	#define BUZ_DEBUG_BREAK() __builtin_trap()
#endif

namespace buz
{
	namespace internal
	{
		//----------------------------------------------------------
		/// @brief Reports a failed assert (logs it), then returns so the
		/// macro can break into the debugger at the call site.
		///
		/// The defaulted location parameter is evaluated where the call is
		/// written, and the assert macros expand at their call site, so it
		/// captures the file, line and function of the failing assert (what
		/// __FILE__ and __LINE__ used to do, plus the function name).
		void on_assert_failed(const char* condition, std::string_view message, std::source_location location = std::source_location::current());

		//----------------------------------------------------------
		/// @brief Reports an unrecoverable error and terminates the program.
		[[noreturn]] void on_fatal_error(std::string_view message);
	}

	//--------------------------------------------------------------
	/// @brief Reports an unrecoverable error and terminates the program:
	/// logs the message, shows a message box, then exits. Works in both
	/// debug and release builds (std::format "{}" style formatting).
	/// @param format Format string with "{}" placeholders, checked at compile time.
	/// @param args Values inserted into the placeholders.
	template<typename... Args>
	[[noreturn]] void fatal_error(std::format_string<Args...> format, Args&&... args)
	{
		internal::on_fatal_error(std::format(format, std::forward<Args>(args)...));
	}
}

//--------------------------------------------------------------
/// @brief Halts the program in the debugger when the condition is false.
///
/// Use it to document assumptions your code makes:
/// @code{.cpp}
/// BUZ_ASSERT(frame_index < frame_count);
/// @endcode
/// Compiled away in release builds.
///
/// A macro (instead of a function) is still needed here: only a macro can
/// stringize the condition with the # operator, skip evaluating it entirely in
/// release, and fire BUZ_DEBUG_BREAK() at the call site instead of inside a
/// helper function.
#if defined(BUZ_ASSERT_ENABLED)
	#define BUZ_ASSERT(condition)                                \
		do                                                       \
		{                                                        \
			if (!(condition))                                    \
			{                                                    \
				buz::internal::on_assert_failed(#condition, {}); \
				BUZ_DEBUG_BREAK();                               \
			}                                                    \
		} while (false)
#else
	#define BUZ_ASSERT(condition) ((void)0)
#endif

//--------------------------------------------------------------
/// @brief Same as BUZ_ASSERT, with an extra message explaining the failure
/// (std::format "{}" style formatting):
/// @code{.cpp}
/// BUZ_ASSERT_MSG(frame_index < frame_count, "frame {} out of range", frame_index);
/// @endcode
#if defined(BUZ_ASSERT_ENABLED)
	#define BUZ_ASSERT_MSG(condition, ...)                                             \
		do                                                                             \
		{                                                                              \
			if (!(condition))                                                          \
			{                                                                          \
				buz::internal::on_assert_failed(#condition, std::format(__VA_ARGS__)); \
				BUZ_DEBUG_BREAK();                                                     \
			}                                                                          \
		} while (false)
#else
	#define BUZ_ASSERT_MSG(condition, ...) ((void)0)
#endif

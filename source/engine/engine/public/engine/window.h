/// @file
/// @brief The window the game draws into.

#pragma once

#include "core/math.h"

namespace buz
{
	class native_window;

	//--------------------------------------------------------------
	/// @brief The window the game draws into. It keeps the size it opened
	/// with (engine_settings::window_width and window_height):
	/// @code{.cpp}
	/// const buz::vec2 size = engine.get_window().get_size();
	/// @endcode
	class window
	{
	public:
		//----------------------------------------------------------
		/// @brief Reads the engine's window; only the engine has one to pass in.
		explicit window(const native_window& native);

		window(const window&) = delete;
		window& operator=(const window&) = delete;

		//----------------------------------------------------------
		/// @brief Returns the size of the drawable area, in pixels.
		vec2 get_size() const;

	private:
		const native_window& m_native; // the engine's side: the SDL window
	};
}

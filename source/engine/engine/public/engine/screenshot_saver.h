/// @file
/// @brief Saving frames as png files.

#pragma once

#include <string_view>

namespace buz
{
	class screenshot;

	//--------------------------------------------------------------
	/// @brief Saves the frame being drawn as a png once it is finished,
	/// e.g. for your learning log.
	///
	/// The engine owns one instance (engine::get_screenshots):
	/// @code{.cpp}
	/// if (input.was_key_pressed(buz::key::f12))
	/// {
	///     engine.get_screenshots().take("screenshots/level_1.png");
	/// }
	/// @endcode
	class screenshot_saver
	{
	public:
		//----------------------------------------------------------
		/// @brief Asks the engine's screenshot for frames; only the engine
		/// has one to pass in.
		explicit screenshot_saver(screenshot& capture);

		screenshot_saver(const screenshot_saver&) = delete;
		screenshot_saver& operator=(const screenshot_saver&) = delete;

		//----------------------------------------------------------
		/// @brief Saves this frame once it is drawn.
		/// @param path Where the png goes; a relative path ends up in the
		/// game's saved/<game> folder next to the executable.
		void take(std::string_view path);

	private:
		screenshot& m_capture; // the engine's side: copies the frame and writes the png
	};
}

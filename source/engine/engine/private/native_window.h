#pragma once

#include <cstdint>
#include <string>

// Defined in SDL3/SDL_video.h; only the engine and the graphics backends
// need its contents.
struct SDL_Window;

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Which graphics api will draw into the window: Vulkan and
	/// OpenGL need a window SDL prepared for them, DirectX 12 does not.
	enum class window_graphics
	{
		native, // DirectX 12: draws into the plain window
		vulkan,
		opengl,
	};

	//--------------------------------------------------------------
	/// @brief The engine's side of the window: the fixed-size SDL window
	/// the engine renders into. Games read its size through buz::window.
	///
	/// A thin layer over an SDL window (https://wiki.libsdl.org/SDL3); SDL
	/// lets the same code run on Windows, on Linux and in the browser.
	class native_window
	{
	public:
		native_window();
		~native_window();

		native_window(const native_window&) = delete;
		native_window& operator=(const native_window&) = delete;

		//----------------------------------------------------------
		/// @brief Starts SDL and opens the window.
		/// @param width Width of the drawable area in pixels.
		/// @param height Height of the drawable area in pixels.
		/// @param title Text shown in the title bar (utf-8).
		/// @param graphics Which graphics api will draw into the window.
		/// @return True when the window was created successfully.
		bool initialize(uint32_t width, uint32_t height, const std::string& title, window_graphics graphics);

		//----------------------------------------------------------
		/// @brief Returns the underlying SDL window.
		SDL_Window* get_sdl_window() const;

		//----------------------------------------------------------
		/// @brief Returns the width of the drawable area in pixels.
		uint32_t get_width() const;

		//----------------------------------------------------------
		/// @brief Returns the height of the drawable area in pixels.
		uint32_t get_height() const;

		//----------------------------------------------------------
		/// @brief Hides the cursor and keeps it in the window (SDL's
		/// relative mouse mode).
		void lock_cursor();

		//----------------------------------------------------------
		/// @brief Shows the cursor again and lets it leave the window.
		void unlock_cursor();

		//----------------------------------------------------------
		/// @brief Returns true while the cursor is locked.
		bool is_cursor_locked() const;

	private:
		SDL_Window* m_window; // owned: destroyed with the window; nullptr until initialize

		uint32_t m_width;  // of the drawable area, in pixels
		uint32_t m_height; // of the drawable area, in pixels
	};
}

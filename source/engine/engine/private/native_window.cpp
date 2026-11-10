#include "native_window.h"

#include "core/log.h"

#include <SDL3/SDL.h>

namespace buz
{
	//--------------------------------------------------------------
	native_window::native_window()
		: m_window(nullptr)
		, m_width(0)
		, m_height(0)
	{
	}

	//--------------------------------------------------------------
	native_window::~native_window()
	{
		if (m_window != nullptr)
		{
			SDL_DestroyWindow(m_window);
		}

		if (SDL_WasInit(0) != 0)
		{
			SDL_Quit();
		}
	}

	//--------------------------------------------------------------
	bool native_window::initialize(uint32_t width, uint32_t height, const std::string& title, window_graphics graphics)
	{
		m_width = width;
		m_height = height;

		if (!SDL_Init(SDL_INIT_VIDEO))
		{
			log::error("Failed to start SDL: {}", SDL_GetError());
			return false;
		}

		// Fixed size: the window has no resize border, so the swap chain
		// never has to be resized. A Vulkan swap chain needs a window SDL
		// prepared for it (DirectX needs nothing special).
		SDL_WindowFlags flags = 0;
		if (graphics == window_graphics::vulkan)
		{
			flags = SDL_WINDOW_VULKAN;
		}
		else if (graphics == window_graphics::opengl)
		{
			// OpenGL draws into the window's own buffers, which are picked
			// when the window is made: say which OpenGL and which buffers
			// first. The rhi creates the OpenGL context itself.
#if defined(__EMSCRIPTEN__)
			// A browser has OpenGL ES 3.0 (WebGL 2)
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
#else
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
			SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
#endif
			SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
			SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
			SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
			flags = SDL_WINDOW_OPENGL;
		}

		m_window = SDL_CreateWindow(title.c_str(), static_cast<int>(width), static_cast<int>(height), flags);
		if (m_window == nullptr)
		{
			log::error("Failed to create the window: {}", SDL_GetError());
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------
	SDL_Window* native_window::get_sdl_window() const
	{
		return m_window;
	}

	//--------------------------------------------------------------
	uint32_t native_window::get_width() const
	{
		return m_width;
	}

	//--------------------------------------------------------------
	uint32_t native_window::get_height() const
	{
		return m_height;
	}
}

#include "opengl/gl_call.h"

#include "core/log.h"

#include <SDL3/SDL_video.h>

namespace buz::rhi::gl
{
#define BUZ_DEFINE_GL_FUNCTION(type, name) type name = nullptr;
	BUZ_GL_FUNCTIONS(BUZ_DEFINE_GL_FUNCTION)
#undef BUZ_DEFINE_GL_FUNCTION

	//--------------------------------------------------------------
	bool load_functions()
	{
		bool all_found = true;

#define BUZ_LOAD_GL_FUNCTION(type, name)                         \
	name = reinterpret_cast<type>(SDL_GL_GetProcAddress(#name)); \
	if (name == nullptr)                                         \
	{                                                            \
		log::error("The OpenGL driver has no function " #name);  \
		all_found = false;                                       \
	}
		BUZ_GL_FUNCTIONS(BUZ_LOAD_GL_FUNCTION)
#undef BUZ_LOAD_GL_FUNCTION

		return all_found;
	}

	//--------------------------------------------------------------
	bool check_errors(const char* call, const char* file, int line)
	{
		// OpenGL may hold several errors; reading one clears it
		bool no_errors = true;
		for (GLenum error = glGetError(); error != GL_NO_ERROR; error = glGetError())
		{
			log::error("OpenGL error 0x{:04X} in {} ({}:{})", error, call, file, line);
			no_errors = false;
		}
		return no_errors;
	}
}

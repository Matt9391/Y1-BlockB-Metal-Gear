#pragma once

#include "core/assert.h"
#include "opengl/gl_functions.h"

namespace buz::rhi::gl
{
	//--------------------------------------------------------------
	/// @brief Logs every error OpenGL collected since the last check.
	/// @param call The call that was just made, as text.
	/// @return True when there was no error.
	bool check_errors(const char* call, const char* file, int line);
}

//--------------------------------------------------------------
// BUZ_GL_CALL(glSomething(...)) makes the call and, in Debug builds, asks
// OpenGL whether it failed. OpenGL never reports an error by itself: it
// only sets a flag that glGetError reads (and clears). Checking after every
// call points straight at the call that went wrong. Release builds skip the
// check, as it makes the cpu wait for the driver.
//--------------------------------------------------------------
#if defined(BUZ_ASSERT_ENABLED)
	#define BUZ_GL_CALL(call)                                                    \
		do                                                                       \
		{                                                                        \
			call;                                                                \
			BUZ_ASSERT(::buz::rhi::gl::check_errors(#call, __FILE__, __LINE__)); \
		} while (false)
#else
	#define BUZ_GL_CALL(call) \
		do                    \
		{                     \
			call;             \
		} while (false)
#endif

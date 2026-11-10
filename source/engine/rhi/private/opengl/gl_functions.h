#pragma once

// Every OpenGL function the backend calls. OpenGL has no import library
// worth linking: the functions live in the graphics driver, which the
// backend asks for them by name once the context exists (load_functions).
// Unreal Engine's OpenGL backend works the same way.
//
// The lists below are the whole of the OpenGL this backend uses: all of it
// is in OpenGL 3.3 core and, except the desktop list, in OpenGL ES 3.0
// (WebGL 2).

// glcorearb.h includes all of windows.h only to learn the calling
// convention of the driver's functions; name it here instead.
#if defined(_WIN32) && !defined(APIENTRY)
	#define APIENTRY __stdcall
#endif
#include <GL/glcorearb.h>

// X(function pointer type, function name)
#define BUZ_GL_FUNCTIONS(X)                                          \
	/* state and frames */                                           \
	X(PFNGLGETERRORPROC, glGetError)                                 \
	X(PFNGLGETSTRINGPROC, glGetString)                               \
	X(PFNGLGETINTEGERVPROC, glGetIntegerv)                           \
	X(PFNGLENABLEPROC, glEnable)                                     \
	X(PFNGLDISABLEPROC, glDisable)                                   \
	X(PFNGLVIEWPORTPROC, glViewport)                                 \
	X(PFNGLCLEARCOLORPROC, glClearColor)                             \
	X(PFNGLCLEARPROC, glClear)                                       \
	X(PFNGLBLENDFUNCSEPARATEPROC, glBlendFuncSeparate)               \
	X(PFNGLCULLFACEPROC, glCullFace)                                 \
	X(PFNGLFRONTFACEPROC, glFrontFace)                               \
	X(PFNGLDEPTHFUNCPROC, glDepthFunc)                               \
	X(PFNGLDEPTHMASKPROC, glDepthMask)                               \
	X(PFNGLPIXELSTOREIPROC, glPixelStorei)                           \
	X(PFNGLREADPIXELSPROC, glReadPixels)                             \
	/* textures and samplers */                                      \
	X(PFNGLGENTEXTURESPROC, glGenTextures)                           \
	X(PFNGLDELETETEXTURESPROC, glDeleteTextures)                     \
	X(PFNGLBINDTEXTUREPROC, glBindTexture)                           \
	X(PFNGLACTIVETEXTUREPROC, glActiveTexture)                       \
	X(PFNGLTEXIMAGE2DPROC, glTexImage2D)                             \
	X(PFNGLTEXPARAMETERIPROC, glTexParameteri)                       \
	X(PFNGLGENSAMPLERSPROC, glGenSamplers)                           \
	X(PFNGLDELETESAMPLERSPROC, glDeleteSamplers)                     \
	X(PFNGLBINDSAMPLERPROC, glBindSampler)                           \
	X(PFNGLSAMPLERPARAMETERIPROC, glSamplerParameteri)               \
	/* buffers and vertex input */                                   \
	X(PFNGLGENBUFFERSPROC, glGenBuffers)                             \
	X(PFNGLDELETEBUFFERSPROC, glDeleteBuffers)                       \
	X(PFNGLBINDBUFFERPROC, glBindBuffer)                             \
	X(PFNGLBUFFERDATAPROC, glBufferData)                             \
	X(PFNGLBUFFERSUBDATAPROC, glBufferSubData)                       \
	X(PFNGLBINDBUFFERRANGEPROC, glBindBufferRange)                   \
	X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays)                   \
	X(PFNGLDELETEVERTEXARRAYSPROC, glDeleteVertexArrays)             \
	X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray)                   \
	X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray)   \
	X(PFNGLDISABLEVERTEXATTRIBARRAYPROC, glDisableVertexAttribArray) \
	X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer)           \
	X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor)           \
	/* shaders */                                                    \
	X(PFNGLCREATESHADERPROC, glCreateShader)                         \
	X(PFNGLSHADERSOURCEPROC, glShaderSource)                         \
	X(PFNGLCOMPILESHADERPROC, glCompileShader)                       \
	X(PFNGLGETSHADERIVPROC, glGetShaderiv)                           \
	X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog)                 \
	X(PFNGLDELETESHADERPROC, glDeleteShader)                         \
	X(PFNGLCREATEPROGRAMPROC, glCreateProgram)                       \
	X(PFNGLATTACHSHADERPROC, glAttachShader)                         \
	X(PFNGLLINKPROGRAMPROC, glLinkProgram)                           \
	X(PFNGLGETPROGRAMIVPROC, glGetProgramiv)                         \
	X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog)               \
	X(PFNGLDELETEPROGRAMPROC, glDeleteProgram)                       \
	X(PFNGLUSEPROGRAMPROC, glUseProgram)                             \
	X(PFNGLGETUNIFORMBLOCKINDEXPROC, glGetUniformBlockIndex)         \
	X(PFNGLUNIFORMBLOCKBINDINGPROC, glUniformBlockBinding)           \
	X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation)             \
	X(PFNGLUNIFORM1IPROC, glUniform1i)                               \
	/* drawing */                                                    \
	X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced)           \
	X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced)       \
	BUZ_GL_DESKTOP_FUNCTIONS(X)

// Desktop OpenGL only: a browser has none of these
#if defined(__EMSCRIPTEN__)
	#define BUZ_GL_DESKTOP_FUNCTIONS(X)
#else
	#define BUZ_GL_DESKTOP_FUNCTIONS(X) \
		X(PFNGLPOLYGONMODEPROC, glPolygonMode)
#endif

namespace buz::rhi::gl
{
// One function pointer per entry of the list, named like the function
#define BUZ_DECLARE_GL_FUNCTION(type, name) extern type name;
	BUZ_GL_FUNCTIONS(BUZ_DECLARE_GL_FUNCTION)
#undef BUZ_DECLARE_GL_FUNCTION

	//--------------------------------------------------------------
	/// @brief Asks the driver for every function in the list; needs a
	/// current OpenGL context.
	/// @return False when a function is missing (each one is logged).
	bool load_functions();
}

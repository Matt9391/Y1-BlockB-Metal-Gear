#pragma once

#include "opengl/gl_functions.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief An OpenGL texture object.
	class gpu_texture::native
	{
	public:
		native(device::native& owner, GLuint texture);
		~native();

		//----------------------------------------------------------
		/// @brief Returns the OpenGL texture object.
		GLuint get_texture() const;

	private:
		device::native& m_owner;
		GLuint m_texture;
	};
}

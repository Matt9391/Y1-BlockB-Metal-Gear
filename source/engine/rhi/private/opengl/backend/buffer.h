#pragma once

#include "opengl/gl_functions.h"
#include "rhi/buffer.h"
#include "rhi/device.h"

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief An OpenGL buffer object holding vertices or indices.
	class buffer::native
	{
	public:
		native(device::native& owner, GLuint buffer, buffer_usage usage, uint32_t stride);
		~native();

		//----------------------------------------------------------
		/// @brief Returns the OpenGL buffer object.
		GLuint get_buffer() const;

		//----------------------------------------------------------
		/// @brief Returns what the buffer holds.
		buffer_usage get_usage() const;

		//----------------------------------------------------------
		/// @brief Returns the size of one vertex in bytes.
		uint32_t get_stride() const;

	private:
		device::native& m_owner;
		GLuint m_buffer;
		buffer_usage m_usage;
		uint32_t m_stride;
	};
}

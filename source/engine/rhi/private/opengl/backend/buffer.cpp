#include "opengl/backend/buffer.h"

#include "opengl/backend/device.h"
#include "opengl/gl_call.h"

using namespace buz::rhi::gl;

namespace buz::rhi
{
	// rhi::buffer hands every call to its native object below. Only
	// this backend's files are compiled (see rhi/CMakeLists.txt).

	//--------------------------------------------------------------
	buffer::buffer(std::unique_ptr<native> data)
		: m_native(std::move(data))
	{
	}

	//--------------------------------------------------------------
	buffer::~buffer() = default;

	//--------------------------------------------------------------
	const buffer::native& buffer::get_native() const
	{
		return *m_native;
	}

	//--------------------------------------------------------------
	buffer::native::native(device::native& owner, GLuint buffer, buffer_usage usage, uint32_t stride)
		: m_owner(owner)
		, m_buffer(buffer)
		, m_usage(usage)
		, m_stride(stride)
	{
		m_owner.resource_created();
	}

	//--------------------------------------------------------------
	buffer::native::~native()
	{
		m_owner.release([buffer = m_buffer] { BUZ_GL_CALL(glDeleteBuffers(1, &buffer)); });
	}

	//--------------------------------------------------------------
	GLuint buffer::native::get_buffer() const
	{
		return m_buffer;
	}

	//--------------------------------------------------------------
	buffer_usage buffer::native::get_usage() const
	{
		return m_usage;
	}

	//--------------------------------------------------------------
	uint32_t buffer::native::get_stride() const
	{
		return m_stride;
	}
}

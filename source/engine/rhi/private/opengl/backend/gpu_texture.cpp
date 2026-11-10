#include "opengl/backend/gpu_texture.h"

#include "opengl/backend/device.h"
#include "opengl/gl_call.h"

using namespace buz::rhi::gl;

namespace buz::rhi
{
	// rhi::gpu_texture hands every call to its native object below. Only
	// this backend's files are compiled (see rhi/CMakeLists.txt).

	//--------------------------------------------------------------
	gpu_texture::gpu_texture(uint32_t width, uint32_t height, std::unique_ptr<native> data)
		: texture(width, height)
		, m_native(std::move(data))
	{
	}

	//--------------------------------------------------------------
	gpu_texture::~gpu_texture() = default;

	//--------------------------------------------------------------
	const gpu_texture::native& gpu_texture::get_native() const
	{
		return *m_native;
	}

	//--------------------------------------------------------------
	gpu_texture::native::native(device::native& owner, GLuint texture)
		: m_owner(owner)
		, m_texture(texture)
	{
		m_owner.resource_created();
	}

	//--------------------------------------------------------------
	gpu_texture::native::~native()
	{
		m_owner.release([texture = m_texture] { BUZ_GL_CALL(glDeleteTextures(1, &texture)); });
	}

	//--------------------------------------------------------------
	GLuint gpu_texture::native::get_texture() const
	{
		return m_texture;
	}
}

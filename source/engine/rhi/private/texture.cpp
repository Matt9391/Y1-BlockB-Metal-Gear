#include "rhi/texture.h"

#include "rhi/gpu_texture.h"

namespace buz
{
	//--------------------------------------------------------------
	texture::texture(uint32_t width, uint32_t height)
		: m_width(width)
		, m_height(height)
	{
	}

	//--------------------------------------------------------------
	texture::~texture() = default;

	//--------------------------------------------------------------
	uint32_t texture::get_width() const
	{
		return m_width;
	}

	//--------------------------------------------------------------
	uint32_t texture::get_height() const
	{
		return m_height;
	}

	//--------------------------------------------------------------
	vec2 texture::get_size() const
	{
		const float width = static_cast<float>(m_width);
		const float height = static_cast<float>(m_height);
		return { width, height };
	}

	namespace rhi
	{
		//----------------------------------------------------------
		const gpu_texture& to_gpu_texture(const texture& image)
		{
			// The engine makes no other kind of texture
			return static_cast<const gpu_texture&>(image);
		}
	}
}

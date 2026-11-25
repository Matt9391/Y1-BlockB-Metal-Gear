#include "render_core/texture.h"

#include "core/assert.h"
#include "rhi/gpu_texture.h"

namespace buz
{
	//--------------------------------------------------------------
	texture::texture(std::unique_ptr<rhi::gpu_texture> image)
		: m_image(std::move(image))
	{
		BUZ_ASSERT_MSG(m_image != nullptr, "A texture needs an image");
	}

	//--------------------------------------------------------------
	texture::~texture() = default;

	//--------------------------------------------------------------
	uint32_t texture::get_width() const
	{
		return m_image->get_width();
	}

	//--------------------------------------------------------------
	uint32_t texture::get_height() const
	{
		return m_image->get_height();
	}

	//--------------------------------------------------------------
	vec2 texture::get_size() const
	{
		const float width = static_cast<float>(get_width());
		const float height = static_cast<float>(get_height());
		return { width, height };
	}

	//--------------------------------------------------------------
	const rhi::gpu_texture& texture::get_gpu_texture() const
	{
		return *m_image;
	}
}

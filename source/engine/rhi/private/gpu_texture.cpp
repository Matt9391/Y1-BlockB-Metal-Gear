#include "rhi/gpu_texture.h"

namespace buz::rhi
{
	// The parts of gpu_texture every backend shares; the constructor and
	// the native data are in each backend's backend/gpu_texture.cpp.

	//--------------------------------------------------------------
	uint32_t gpu_texture::get_width() const
	{
		return m_width;
	}

	//--------------------------------------------------------------
	uint32_t gpu_texture::get_height() const
	{
		return m_height;
	}
}

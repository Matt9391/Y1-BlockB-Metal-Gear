#include "d3d12/backend/gpu_texture.h"

#include "d3d12/backend/device.h"
#include "d3d12/descriptor_heap.h"

using Microsoft::WRL::ComPtr;

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
	gpu_texture::native::native(device::native& owner, ComPtr<ID3D12Resource> resource, uint32_t descriptor_index)
		: m_owner(owner)
		, m_resource(std::move(resource))
		, m_descriptor_index(descriptor_index)
	{
		m_owner.resource_created();
	}

	//--------------------------------------------------------------
	gpu_texture::native::~native()
	{
		// The gpu may still sample this texture in a frame in flight: keep the
		// resource (the lambda holds a reference) and the descriptor slot until
		// those frames are done.
		d3d12::descriptor_heap& texture_heap = m_owner.get_texture_heap();
		m_owner.release_later([resource = m_resource, descriptor_index = m_descriptor_index, &texture_heap] {
			texture_heap.free(descriptor_index);
		});
	}

	//--------------------------------------------------------------
	uint32_t gpu_texture::native::get_descriptor_index() const
	{
		return m_descriptor_index;
	}
}

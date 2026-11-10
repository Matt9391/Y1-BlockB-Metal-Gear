#include "d3d12/backend/buffer.h"

#include "d3d12/backend/device.h"

using Microsoft::WRL::ComPtr;

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
	buffer::native::native(device::native& owner, ComPtr<ID3D12Resource> resource, buffer_usage usage, uint32_t size, uint32_t stride)
		: m_owner(owner)
		, m_resource(std::move(resource))
		, m_usage(usage)
		, m_vertex_view{}
		, m_index_view{}
	{
		m_owner.resource_created();

		if (usage == buffer_usage::vertex)
		{
			m_vertex_view = { m_resource->GetGPUVirtualAddress(), size, stride };
		}
		else
		{
			m_index_view = { m_resource->GetGPUVirtualAddress(), size, usage == buffer_usage::index32 ? DXGI_FORMAT_R32_UINT : DXGI_FORMAT_R16_UINT };
		}
	}

	//--------------------------------------------------------------
	buffer::native::~native()
	{
		m_owner.release_later([resource = m_resource] {});
	}

	//--------------------------------------------------------------
	buffer_usage buffer::native::get_usage() const
	{
		return m_usage;
	}

	//--------------------------------------------------------------
	const D3D12_VERTEX_BUFFER_VIEW& buffer::native::get_vertex_view() const
	{
		return m_vertex_view;
	}

	//--------------------------------------------------------------
	const D3D12_INDEX_BUFFER_VIEW& buffer::native::get_index_view() const
	{
		return m_index_view;
	}
}

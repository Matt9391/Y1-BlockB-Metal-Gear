#pragma once

#include "rhi/buffer.h"
#include "rhi/device.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief A vertex or index buffer.
	///
	/// Buffers live in an upload heap: cpu memory the gpu reads directly.
	/// That keeps them simple and is plenty fast for the small buffers buz
	/// draws with; big meshes would be copied to gpu memory, like textures.
	class buffer::native
	{
	public:
		native(device::native& owner, Microsoft::WRL::ComPtr<ID3D12Resource> resource, buffer_usage usage, uint32_t size, uint32_t stride);
		~native();

		//----------------------------------------------------------
		/// @brief Returns what the buffer holds.
		buffer_usage get_usage() const;

		//----------------------------------------------------------
		/// @brief Returns how the input assembler reads it as vertices.
		const D3D12_VERTEX_BUFFER_VIEW& get_vertex_view() const;

		//----------------------------------------------------------
		/// @brief Returns how the input assembler reads it as indices.
		const D3D12_INDEX_BUFFER_VIEW& get_index_view() const;

	private:
		device::native& m_owner;                           // creates and releases the api objects
		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource; // the gpu memory
		buffer_usage m_usage;                              // what the buffer holds
		D3D12_VERTEX_BUFFER_VIEW m_vertex_view;            // filled in for a vertex buffer
		D3D12_INDEX_BUFFER_VIEW m_index_view;              // filled in for an index buffer
	};
}

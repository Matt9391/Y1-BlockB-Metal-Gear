#pragma once

#include "rhi/device.h"
#include "rhi/gpu_texture.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief A texture in gpu memory plus its shader resource view (the
	/// descriptor shaders use to sample it).
	class gpu_texture::native
	{
	public:
		native(device::native& owner, Microsoft::WRL::ComPtr<ID3D12Resource> resource, uint32_t descriptor_index);
		~native();

		//----------------------------------------------------------
		/// @brief Returns the slot of the texture descriptor (shader resource view) in the texture heap.
		uint32_t get_descriptor_index() const;

	private:
		device::native& m_owner;                           // creates and releases the api objects
		Microsoft::WRL::ComPtr<ID3D12Resource> m_resource; // the gpu memory
		uint32_t m_descriptor_index;                       // its shader resource view in the texture heap
	};
}

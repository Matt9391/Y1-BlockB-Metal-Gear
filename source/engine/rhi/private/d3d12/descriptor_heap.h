#pragma once

#include <d3d12.h>
#include <wrl/client.h>

#include <cstdint>
#include <vector>

namespace buz::d3d12
{
	//--------------------------------------------------------------
	/// @brief A fixed-size descriptor heap that hands out slots one at a time.
	///
	/// A descriptor is a small block of gpu-readable data that describes a
	/// resource (for example a texture). Descriptors live in descriptor
	/// heaps. Freed slots are handed out again, so games that create and
	/// destroy textures while running do not run out.
	class descriptor_heap
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates an empty heap; initialize creates it.
		descriptor_heap();

		//----------------------------------------------------------
		/// @brief Creates the descriptor heap.
		/// @param d3d_device Device used to create the heap.
		/// @param type Type of descriptors stored in the heap.
		/// @param capacity Maximum number of descriptors in the heap.
		/// @param shader_visible True when shaders need to read from this heap.
		/// @return True when the heap was created successfully.
		bool initialize(ID3D12Device* d3d_device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible);

		//----------------------------------------------------------
		/// @brief Reserves a free descriptor slot.
		/// @return Index of the reserved slot.
		uint32_t allocate();

		//----------------------------------------------------------
		/// @brief Gives a slot back so allocate can hand it out again.
		/// @param index Slot index returned by allocate().
		void free(uint32_t index);

		//----------------------------------------------------------
		/// @brief Returns the cpu handle for a descriptor slot (used to create descriptors).
		/// @param index Slot index returned by allocate().
		D3D12_CPU_DESCRIPTOR_HANDLE get_cpu_handle(uint32_t index) const;

		//----------------------------------------------------------
		/// @brief Returns the gpu handle for a descriptor slot (used while drawing).
		/// @param index Slot index returned by allocate().
		D3D12_GPU_DESCRIPTOR_HANDLE get_gpu_handle(uint32_t index) const;

		//----------------------------------------------------------
		/// @brief Returns the underlying directx 12 descriptor heap.
		ID3D12DescriptorHeap* get_d3d_heap() const;

	private:
		Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> m_heap; // the descriptors

		uint32_t m_descriptor_size;         // bytes from one descriptor to the next
		uint32_t m_capacity;                // how many descriptors fit
		uint32_t m_allocated_count;         // how many were ever handed out
		std::vector<uint32_t> m_free_slots; // slots handed back, reused first
	};
}

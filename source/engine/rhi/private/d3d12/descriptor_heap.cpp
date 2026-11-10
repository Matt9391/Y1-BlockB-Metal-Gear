#include "d3d12/descriptor_heap.h"

#include "core/log.h"
#include "d3d12/d3d_utils.h"

namespace buz::d3d12
{
	//--------------------------------------------------------------
	descriptor_heap::descriptor_heap()
		: m_descriptor_size(0)
		, m_capacity(0)
		, m_allocated_count(0)
	{
	}

	//--------------------------------------------------------------
	bool descriptor_heap::initialize(ID3D12Device* d3d_device, D3D12_DESCRIPTOR_HEAP_TYPE type, uint32_t capacity, bool shader_visible)
	{
		const D3D12_DESCRIPTOR_HEAP_DESC heap_desc = {
			.Type = type,
			.NumDescriptors = capacity,
			.Flags = shader_visible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
		};

		if (!d3d::verify(d3d_device->CreateDescriptorHeap(&heap_desc, IID_PPV_ARGS(&m_heap)), "CreateDescriptorHeap"))
		{
			return false;
		}

		// Descriptor sizes differ per gpu vendor, so they have to be queried
		m_descriptor_size = d3d_device->GetDescriptorHandleIncrementSize(type);
		m_capacity = capacity;
		m_allocated_count = 0;

		return true;
	}

	//--------------------------------------------------------------
	uint32_t descriptor_heap::allocate()
	{
		if (!m_free_slots.empty())
		{
			const uint32_t index = m_free_slots.back();
			m_free_slots.pop_back();
			return index;
		}

		if (m_allocated_count >= m_capacity)
		{
			log::error("Descriptor heap is full (capacity: {})", m_capacity);
			return 0;
		}

		return m_allocated_count++;
	}

	//--------------------------------------------------------------
	D3D12_CPU_DESCRIPTOR_HANDLE descriptor_heap::get_cpu_handle(uint32_t index) const
	{
		D3D12_CPU_DESCRIPTOR_HANDLE handle = m_heap->GetCPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<SIZE_T>(index) * m_descriptor_size;

		return handle;
	}

	//--------------------------------------------------------------
	D3D12_GPU_DESCRIPTOR_HANDLE descriptor_heap::get_gpu_handle(uint32_t index) const
	{
		D3D12_GPU_DESCRIPTOR_HANDLE handle = m_heap->GetGPUDescriptorHandleForHeapStart();
		handle.ptr += static_cast<UINT64>(index) * m_descriptor_size;

		return handle;
	}

	//--------------------------------------------------------------
	void descriptor_heap::free(uint32_t index)
	{
		m_free_slots.push_back(index);
	}

	//--------------------------------------------------------------
	ID3D12DescriptorHeap* descriptor_heap::get_d3d_heap() const
	{
		return m_heap.Get();
	}
}

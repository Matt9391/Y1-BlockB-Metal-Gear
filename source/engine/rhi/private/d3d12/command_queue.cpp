#include "d3d12/command_queue.h"

#include "core/log.h"
#include "d3d12/d3d_utils.h"

namespace buz::d3d12
{
	//--------------------------------------------------------------
	command_queue::command_queue()
		: m_fence_event(nullptr)
		, m_next_fence_value(1)
	{
	}

	//--------------------------------------------------------------
	bool command_queue::initialize(ID3D12Device* d3d_device, D3D12_COMMAND_LIST_TYPE type)
	{
		const D3D12_COMMAND_QUEUE_DESC queue_desc = { .Type = type };

		if (!d3d::verify(d3d_device->CreateCommandQueue(&queue_desc, IID_PPV_ARGS(&m_queue)), "CreateCommandQueue"))
		{
			return false;
		}

		if (!d3d::verify(d3d_device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&m_fence)), "CreateFence"))
		{
			return false;
		}

		m_fence_event = CreateEventW(nullptr, FALSE, FALSE, nullptr);
		if (m_fence_event == nullptr)
		{
			log::error("Failed to create the fence event handle");
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------
	command_queue::~command_queue()
	{
		if (m_fence_event != nullptr)
		{
			CloseHandle(m_fence_event);
		}
	}

	//--------------------------------------------------------------
	uint64_t command_queue::submit(ID3D12CommandList* command_list)
	{
		ID3D12CommandList* command_lists[] = { command_list };
		m_queue->ExecuteCommandLists(1, command_lists);

		return signal();
	}

	//--------------------------------------------------------------
	uint64_t command_queue::signal()
	{
		const uint64_t fence_value = m_next_fence_value++;
		m_queue->Signal(m_fence.Get(), fence_value);

		return fence_value;
	}

	//--------------------------------------------------------------
	uint64_t command_queue::get_completed_value() const
	{
		return m_fence->GetCompletedValue();
	}

	//--------------------------------------------------------------
	void command_queue::wait_for_fence_value(uint64_t fence_value)
	{
		if (m_fence->GetCompletedValue() >= fence_value)
		{
			return;
		}

		m_fence->SetEventOnCompletion(fence_value, m_fence_event);
		WaitForSingleObject(m_fence_event, INFINITE);
	}

	//--------------------------------------------------------------
	void command_queue::flush()
	{
		// Nothing to wait for when initialization never got this far
		if (m_fence == nullptr)
		{
			return;
		}

		wait_for_fence_value(signal());
	}

	//--------------------------------------------------------------
	ID3D12CommandQueue* command_queue::get_d3d_command_queue() const
	{
		return m_queue.Get();
	}

	//--------------------------------------------------------------
	uint64_t command_queue::get_next_fence_value() const
	{
		return m_next_fence_value;
	}
}

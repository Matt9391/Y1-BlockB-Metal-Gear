#include "d3d12/d3d_utils.h"

#include "core/log.h"

namespace buz::d3d
{
	//--------------------------------------------------------------
	bool verify(HRESULT result, const char* what)
	{
		if (FAILED(result))
		{
			log::error("{} failed (hresult 0x{:08X})", what, static_cast<unsigned int>(result));
			return false;
		}

		return true;
	}

	//--------------------------------------------------------------
	debug_break_guard::debug_break_guard(ID3D12Device* device, D3D12_MESSAGE_SEVERITY severity, BOOL break_enabled, bool enabled)
#if defined(_DEBUG)
		: m_severity(severity)
		, m_break_on_severity(FALSE)
#endif
	{
#if defined(_DEBUG)
		if (enabled && SUCCEEDED(device->QueryInterface(IID_PPV_ARGS(&m_info_queue))))
		{
			m_break_on_severity = m_info_queue->GetBreakOnSeverity(severity);
			m_info_queue->SetBreakOnSeverity(severity, break_enabled);
		}
#else
		(void)device;
		(void)severity;
		(void)break_enabled;
		(void)enabled;
#endif
	}

	//--------------------------------------------------------------
	debug_break_guard::~debug_break_guard()
	{
#if defined(_DEBUG)
		if (m_info_queue != nullptr)
		{
			m_info_queue->SetBreakOnSeverity(m_severity, m_break_on_severity);
		}
#endif
	}

	//--------------------------------------------------------------
	D3D12_RESOURCE_BARRIER make_transition_barrier(ID3D12Resource* resource, D3D12_RESOURCE_STATES state_before, D3D12_RESOURCE_STATES state_after)
	{
		// Designated initializers (C++20) name each field they set; every
		// field not named stays zero. .Transition picks which member of
		// the barrier's union this initializer fills in.
		return D3D12_RESOURCE_BARRIER{
			.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
			.Transition = {
				.pResource = resource,
				.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
				.StateBefore = state_before,
				.StateAfter = state_after,
			},
		};
	}

	//--------------------------------------------------------------
	D3D12_RESOURCE_DESC make_buffer_desc(UINT64 size)
	{
		return D3D12_RESOURCE_DESC{
			.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
			.Width = size,
			.Height = 1,
			.DepthOrArraySize = 1,
			.MipLevels = 1,
			.Format = DXGI_FORMAT_UNKNOWN,
			.SampleDesc = { .Count = 1 },
			.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
		};
	}

	//--------------------------------------------------------------
	D3D12_HEAP_PROPERTIES make_heap_properties(D3D12_HEAP_TYPE type)
	{
		return D3D12_HEAP_PROPERTIES{ .Type = type };
	}
}

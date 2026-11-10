#include "d3d12/swap_chain.h"

#include "d3d12/command_queue.h"
#include "d3d12/d3d_utils.h"
#include "d3d12/device.h"

using Microsoft::WRL::ComPtr;

namespace buz::d3d12
{
	//--------------------------------------------------------------
	swap_chain::swap_chain()
		: m_vsync(vsync::yes)
		, m_width(0)
		, m_height(0)
	{
	}

	//--------------------------------------------------------------
	bool swap_chain::initialize(device& graphics_device, command_queue& direct_queue, HWND window_handle, uint32_t width, uint32_t height, vsync wait_for_vsync)
	{
		m_width = width;
		m_height = height;
		m_vsync = wait_for_vsync;

		const DXGI_SWAP_CHAIN_DESC1 swap_chain_desc = {
			.Width = width,
			.Height = height,
			.Format = k_back_buffer_format,
			.SampleDesc = { .Count = 1 },
			.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
			.BufferCount = k_back_buffer_count,
			.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD,
		};

		IDXGIFactory4* factory = graphics_device.get_dxgi_factory();

		ComPtr<IDXGISwapChain1> swap_chain;
		if (!d3d::verify(factory->CreateSwapChainForHwnd(direct_queue.get_d3d_command_queue(), window_handle, &swap_chain_desc, nullptr, nullptr, &swap_chain), "CreateSwapChainForHwnd"))
		{
			return false;
		}

		if (!d3d::verify(swap_chain.As(&m_swap_chain), "IDXGISwapChain1::QueryInterface"))
		{
			return false;
		}

		// The engine has no fullscreen support, so disable alt+enter
		factory->MakeWindowAssociation(window_handle, DXGI_MWA_NO_ALT_ENTER);

		// Create a render target view for every back buffer so we can render into them
		if (!m_rtv_heap.initialize(graphics_device.get_d3d_device(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, k_back_buffer_count, false))
		{
			return false;
		}

		for (uint32_t index = 0; index < k_back_buffer_count; ++index)
		{
			if (!d3d::verify(m_swap_chain->GetBuffer(index, IID_PPV_ARGS(&m_back_buffers[index])), "IDXGISwapChain3::GetBuffer"))
			{
				return false;
			}

			const uint32_t rtv_index = m_rtv_heap.allocate();
			graphics_device.get_d3d_device()->CreateRenderTargetView(m_back_buffers[index].Get(), nullptr, m_rtv_heap.get_cpu_handle(rtv_index));
		}

		return true;
	}

	//--------------------------------------------------------------
	uint32_t swap_chain::acquire_next_image() const
	{
		return m_swap_chain->GetCurrentBackBufferIndex();
	}

	//--------------------------------------------------------------
	bool swap_chain::present()
	{
		return d3d::verify(m_swap_chain->Present(m_vsync ? 1 : 0, 0), "IDXGISwapChain3::Present");
	}

	//--------------------------------------------------------------
	ID3D12Resource* swap_chain::get_image(uint32_t index) const
	{
		return m_back_buffers[index].Get();
	}

	//--------------------------------------------------------------
	D3D12_CPU_DESCRIPTOR_HANDLE swap_chain::get_render_target_view(uint32_t index) const
	{
		return m_rtv_heap.get_cpu_handle(index);
	}

	//--------------------------------------------------------------
	uint32_t swap_chain::get_width() const
	{
		return m_width;
	}

	//--------------------------------------------------------------
	uint32_t swap_chain::get_height() const
	{
		return m_height;
	}
}

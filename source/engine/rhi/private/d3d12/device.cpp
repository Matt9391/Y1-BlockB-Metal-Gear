#include "d3d12/device.h"

#include "core/log.h"
#include "d3d12/d3d_utils.h"
#include "d3d12/shader_model.h"
#include "common/adapter_scoring.h"

using Microsoft::WRL::ComPtr;

namespace
{
	//--------------------------------------------------------------
	/// @brief Enables the directx 12 debug layer in debug builds.
	///
	/// The debug layer validates every api call and prints readable error
	/// messages to the visual studio output window. It is invaluable while
	/// learning directx, but it slows rendering down, so release builds
	/// leave it off.
	void enable_debug_layer()
	{
#if defined(_DEBUG)
		ComPtr<ID3D12Debug> debug_interface;
		if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debug_interface))))
		{
			debug_interface->EnableDebugLayer();
			buz::log::info("Directx 12 debug layer enabled");
		}
#endif
	}

	//--------------------------------------------------------------
	/// @brief Makes the debugger break on directx errors in debug builds.
	void enable_debug_breaks([[maybe_unused]] ID3D12Device* d3d_device)
	{
#if defined(_DEBUG)
		ComPtr<ID3D12InfoQueue> info_queue;
		if (SUCCEEDED(d3d_device->QueryInterface(IID_PPV_ARGS(&info_queue))))
		{
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE);
			info_queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE);
		}
#endif
	}
}

namespace buz::d3d12
{
	//--------------------------------------------------------------
	bool device::initialize(adapter_preference preference)
	{
		enable_debug_layer();

		UINT factory_flags = 0;
#if defined(_DEBUG)
		factory_flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif

		if (!d3d::verify(CreateDXGIFactory2(factory_flags, IID_PPV_ARGS(&m_factory)), "CreateDXGIFactory2"))
		{
			return false;
		}

		// Score all graphics cards on this machine and pick one
		const std::vector<adapter_info> adapters = enumerate_and_score_adapters(m_factory.Get());

		const adapter_info* selected = rhi::select_adapter(adapters, preference);
		if (selected == nullptr)
		{
			log::error("{}", preference == adapter_preference::software ? "No software rasterizer found" : "No directx 12 capable graphics card found");
			return false;
		}

		m_adapter_info = *selected;
		log::info("Selected adapter: {} ({})", m_adapter_info.description, rhi::describe(preference));

		// Create the real device on the selected adapter
		if (!d3d::verify(D3D12CreateDevice(m_adapter_info.adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&m_device)), "D3D12CreateDevice"))
		{
			return false;
		}

		enable_debug_breaks(m_device.Get());

		const unsigned int feature_level = static_cast<unsigned int>(m_adapter_info.max_feature_level);
		const std::string shader_model = shader_model_to_string(m_adapter_info.highest_shader_model);
		log::info("Device created (feature level: {:X}, shader model: {})", feature_level, shader_model);

		return true;
	}

	//--------------------------------------------------------------
	ID3D12Device* device::get_d3d_device() const
	{
		return m_device.Get();
	}

	//--------------------------------------------------------------
	IDXGIFactory4* device::get_dxgi_factory() const
	{
		return m_factory.Get();
	}

	//--------------------------------------------------------------
	const adapter_info& device::get_adapter_info() const
	{
		return m_adapter_info;
	}
}

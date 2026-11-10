#pragma once

#include "d3d12/adapter_selection.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

namespace buz::d3d12
{
	//--------------------------------------------------------------
	/// @brief Owns the directx 12 device and the adapter it was created on.
	///
	/// Initialization enables the debug layer (in debug builds), enumerates
	/// and scores all graphics cards, selects one based on the given
	/// preference and creates the actual device on it.
	class device
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates the directx 12 device.
		/// @param preference Whether to run on the highest or lowest scoring graphics card.
		/// @return True when the device was created successfully.
		bool initialize(adapter_preference preference);

		//----------------------------------------------------------
		/// @brief Returns the underlying directx 12 device.
		ID3D12Device* get_d3d_device() const;

		//----------------------------------------------------------
		/// @brief Returns the dxgi factory used to create the device (also used for the swap chain).
		IDXGIFactory4* get_dxgi_factory() const;

		//----------------------------------------------------------
		/// @brief Returns information about the graphics card the device runs on.
		const adapter_info& get_adapter_info() const;

	private:
		Microsoft::WRL::ComPtr<IDXGIFactory4> m_factory;
		Microsoft::WRL::ComPtr<ID3D12Device> m_device;

		adapter_info m_adapter_info;
	};
}

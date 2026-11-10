#pragma once

#include "d3d12/descriptor_heap.h"
#include "rhi/settings.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

#include <cstdint>

namespace buz::d3d12
{
	class device;
	class command_queue;

	//--------------------------------------------------------------
	/// @brief Owns the swap chain and its back buffers.
	///
	/// The swap chain holds the images that end up on screen. While the gpu
	/// renders into one back buffer, the other one is being displayed;
	/// present() swaps their roles.
	class swap_chain
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates an empty swap chain; initialize creates it.
		swap_chain();

		// Two buffers: one on screen, one being rendered into
		static constexpr uint32_t k_back_buffer_count = 2;

		//----------------------------------------------------------
		/// @brief Creates the swap chain for a window.
		/// @param graphics_device Device the back buffers are created on.
		/// @param direct_queue Queue that presents frames to the screen.
		/// @param window_handle Window to present into.
		/// @param width Width of the back buffers in pixels.
		/// @param height Height of the back buffers in pixels.
		/// @param wait_for_vsync Whether present waits for the monitor's refresh.
		/// @return True when the swap chain was created successfully.
		bool initialize(device& graphics_device, command_queue& direct_queue, HWND window_handle, uint32_t width, uint32_t height, vsync wait_for_vsync);

		//----------------------------------------------------------
		/// @brief Returns the index of the back buffer to render into this
		/// frame. DirectX hands them out in a fixed order, so there is
		/// nothing to wait for (Vulkan's swap chain has to be asked).
		uint32_t acquire_next_image() const;

		//----------------------------------------------------------
		/// @brief Returns a back buffer resource.
		ID3D12Resource* get_image(uint32_t index) const;

		//----------------------------------------------------------
		/// @brief Returns the render target view of a back buffer.
		D3D12_CPU_DESCRIPTOR_HANDLE get_render_target_view(uint32_t index) const;

		//----------------------------------------------------------
		/// @brief Shows the back buffer rendered this frame on screen.
		/// @return True when presenting succeeded.
		bool present();

		//----------------------------------------------------------
		/// @brief Returns the width of the back buffers in pixels.
		uint32_t get_width() const;

		//----------------------------------------------------------
		/// @brief Returns the height of the back buffers in pixels.
		uint32_t get_height() const;

		// The format all rendering targets, used when creating pipelines
		static constexpr DXGI_FORMAT k_back_buffer_format = DXGI_FORMAT_R8G8B8A8_UNORM;

	private:
		Microsoft::WRL::ComPtr<IDXGISwapChain3> m_swap_chain;                       // shows the back buffers in the window
		Microsoft::WRL::ComPtr<ID3D12Resource> m_back_buffers[k_back_buffer_count]; // drawn into, one at a time

		descriptor_heap m_rtv_heap; // the back buffers' render target views
		vsync m_vsync;              // whether presenting waits for the monitor

		uint32_t m_width;  // of the back buffers, in pixels
		uint32_t m_height; // of the back buffers, in pixels
	};
}

#pragma once

// The DirectX 12 implementation of the rhi: the `native` half of every rhi
// class, one file per class in this folder. Its layout mirrors the Vulkan
// backend (vulkan/backend/) on purpose: the same classes, the same frame loop
// and the same resource lifetime rules, so reading one explains the other and
// a fix in one shows where the other needs it too.

#include "common/constants.h"
#include "common/frame_scratch.h"
#include "common/lifetime_tracker.h"
#include "d3d12/command_queue.h"
#include "d3d12/descriptor_heap.h"
#include "d3d12/device.h"
#include "d3d12/pso_cache.h"
#include "d3d12/swap_chain.h"
#include "rhi/command_list.h"
#include "rhi/device.h"

#include <d3d12.h>
#include <wrl/client.h>

#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief The DirectX 12 device, queue, swap chain and frame loop.
	class device::native
	{
	public:
		native();
		~native();

		//----------------------------------------------------------
		/// @brief Creates the device, queue, swap chain and descriptor heap.
		/// @return True when everything was created successfully.
		bool initialize(const device_desc& desc);

		//----------------------------------------------------------
		// The rhi::device functions (see rhi/device.h)
		std::unique_ptr<gpu_texture> create_texture(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);
		std::unique_ptr<buffer> create_buffer(buffer_usage usage, std::span<const std::byte> data, uint32_t stride);
		std::unique_ptr<pipeline> create_pipeline(const pipeline_desc& desc);

		command_list& begin_frame(color clear_color);
		void end_frame();
		void shutdown();

		frame_stats get_frame_stats() const;
		void request_capture();
		std::vector<uint8_t> read_capture();
		uint32_t get_width() const;
		uint32_t get_height() const;

		//----------------------------------------------------------
		/// @brief Frees a destroyed resource's api objects once the gpu is
		/// done with every frame recorded so far. Called by the resources.
		void release_later(std::function<void()> release);

		//----------------------------------------------------------
		/// @brief Counts a newly created resource. Called by the resources.
		void resource_created();

		//----------------------------------------------------------
		/// @brief Returns the heap holding the texture descriptors.
		d3d12::descriptor_heap& get_texture_heap();

		//----------------------------------------------------------
		/// @brief A piece of the current frame's scratch memory.
		struct scratch_allocation
		{
			std::byte* data = nullptr;                 // where the cpu writes
			D3D12_GPU_VIRTUAL_ADDRESS gpu_address = 0; // where the gpu reads
		};

		//----------------------------------------------------------
		/// @brief Reserves scratch memory in the frame being recorded.
		/// @return The allocation, or nothing when the frame's scratch is full.
		std::optional<scratch_allocation> allocate_scratch(uint32_t size);

	private:
		//----------------------------------------------------------
		/// @brief Everything one frame in flight needs of its own.
		struct frame
		{
			// A command allocator can only be reset once the gpu finished
			// the frame that used it; fence_value says when that is.
			Microsoft::WRL::ComPtr<ID3D12CommandAllocator> allocator;
			Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> command_list;
			uint64_t fence_value = 0;

			// Upload heap memory for data that only lives for this frame,
			// mapped for the whole lifetime of the backend.
			Microsoft::WRL::ComPtr<ID3D12Resource> scratch_buffer;
			std::byte* scratch_data = nullptr;
			frame_scratch scratch;
		};

		//----------------------------------------------------------
		/// @brief Creates the root signature every pipeline shares the
		/// layout of: constants in b0, one texture in t0, a sampler in s0.
		Microsoft::WRL::ComPtr<ID3D12RootSignature> create_root_signature(uint32_t constants_size, texture_filter sampler_filter);

		// Qualified: inside rhi::device, a plain "device" names rhi::device
		d3d12::device m_device;
		d3d12::command_queue m_queue;
		d3d12::swap_chain m_swap_chain;
		d3d12::descriptor_heap m_texture_heap;
		d3d12::pso_cache m_pso_cache;
		lifetime_tracker m_lifetime;

		// One depth buffer, the size of the back buffers, shared by every
		// frame: the queue runs frames one after another, so they never use
		// it at the same time.
		Microsoft::WRL::ComPtr<ID3D12Resource> m_depth_buffer;
		d3d12::descriptor_heap m_depth_heap;

		frame m_frames[k_frames_in_flight]; // used round robin, one per frame in flight
		uint32_t m_frame_index;             // which of m_frames is being recorded
		uint32_t m_image_index;             // which swap chain image it draws into
		command_list m_command_list;        // records the frame being drawn
		frame_stats m_last_frame_stats;     // what the previous frame asked of the gpu

		// Frame capture: a readback heap buffer the finished back buffer is
		// copied into, and how its rows are laid out there.
		bool m_capture_requested;
		Microsoft::WRL::ComPtr<ID3D12Resource> m_capture_buffer;
		D3D12_PLACED_SUBRESOURCE_FOOTPRINT m_capture_footprint;
		uint64_t m_capture_fence_value; // the copy is done once the gpu passed it

		// Reused for every texture upload
		Microsoft::WRL::ComPtr<ID3D12CommandAllocator> m_upload_allocator;
		Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> m_upload_command_list;

		bool m_is_shut_down; // shutdown ran; the destructor does not run it again
	};
}

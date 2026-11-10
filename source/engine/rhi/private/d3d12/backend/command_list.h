#pragma once

#include "rhi/buffer.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"
#include "rhi/pipeline.h"

#include <d3d12.h>

#include <cstddef>
#include <cstdint>
#include <span>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Records draws into the graphics command list of a frame.
	class command_list::native
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates a wrapper that records nothing until begin.
		native();

		//----------------------------------------------------------
		/// @brief Points the wrapper at the command list of this frame.
		void begin(device::native& owner, ID3D12GraphicsCommandList* d3d_command_list);

		//----------------------------------------------------------
		// The rhi::command_list functions (see rhi/command_list.h)
		void set_pipeline(const pipeline::native& active_pipeline);
		void set_vertex_buffer(const buffer::native& vertices);
		bool set_vertex_data(vertex_rate rate, std::span<const std::byte> data, uint32_t stride);
		void set_index_buffer(const buffer::native& indices);
		void set_constants(std::span<const std::byte> constants);
		void set_texture(const gpu_texture::native& sampled_texture);
		void draw(uint32_t vertex_count, uint32_t instance_count);
		void draw_indexed(uint32_t index_count, uint32_t instance_count);

		//----------------------------------------------------------
		/// @brief Returns what was recorded since begin.
		const frame_stats& get_stats() const;

	private:
		device::native* m_owner;                   // not owned; set by begin
		ID3D12GraphicsCommandList* m_command_list; // not owned: the frame's command list
		frame_stats m_stats;                       // what was recorded since begin
	};
}

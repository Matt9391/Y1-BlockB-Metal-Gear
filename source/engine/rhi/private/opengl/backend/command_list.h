#pragma once

#include "opengl/gl_functions.h"
#include "rhi/buffer.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"
#include "rhi/pipeline.h"

#include <cstddef>
#include <cstdint>
#include <span>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief Issues the draws of a frame. OpenGL has no command lists: every
	/// call goes to the driver right away, which queues it for the gpu.
	class command_list::native
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates a command list that issues nothing until begin.
		native();

		//----------------------------------------------------------
		/// @brief Starts recording a frame for this device.
		void begin(device::native& owner);

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
		//----------------------------------------------------------
		/// @brief Where one vertex stream reads from.
		struct vertex_stream
		{
			GLuint buffer = 0;    // the OpenGL buffer object
			uintptr_t offset = 0; // in bytes, from the start of the buffer
			uint32_t stride = 0;  // size of one element in bytes
		};

		//----------------------------------------------------------
		/// @brief Points every vertex shader input at its stream. OpenGL
		/// 3.3 ties the vertex layout to the buffers, so this runs for every
		/// draw, once the pipeline and the buffers are known.
		void bind_vertex_inputs();

		device::native* m_owner;            // not owned; set by begin
		const pipeline::native* m_pipeline; // not owned: the pipeline set last
		vertex_stream m_streams[2];         // 0: per vertex, 1: per instance
		GLenum m_index_type;                // of the index buffer set last
		GLuint m_enabled_inputs;            // how many vertex inputs are switched on
		frame_stats m_stats;                // what was recorded since begin
	};
}

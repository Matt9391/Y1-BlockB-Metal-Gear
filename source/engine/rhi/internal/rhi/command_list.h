#pragma once

#include "rhi/pipeline.h"

#include <cstddef>
#include <cstdint>
#include <memory>
#include <span>

namespace buz::rhi
{
	class buffer;
	class gpu_texture;

	//--------------------------------------------------------------
	/// @brief Records the draw commands of one frame.
	class command_list
	{
	public:
		class native; // the backend's data: the api command list being recorded

		explicit command_list(std::unique_ptr<native> data);
		~command_list();

		//----------------------------------------------------------
		/// @brief Selects the pipeline the next draws use.
		void set_pipeline(const pipeline& active_pipeline);

		//----------------------------------------------------------
		/// @brief Selects the vertices the next draws read (stream 0).
		void set_vertex_buffer(const buffer& vertices);

		//----------------------------------------------------------
		/// @brief Copies data that only lives for this frame (vertices
		/// built on the fly, or one element per instance) into the
		/// frame's scratch memory and selects it for the next draws.
		/// @param rate per_vertex selects stream 0, per_instance stream 1.
		/// @param data The elements, back to back.
		/// @param stride Size of one element in bytes.
		/// @return False when the frame's scratch memory is full (the
		/// draw should be skipped; the error is logged once per frame).
		bool set_vertex_data(vertex_rate rate, std::span<const std::byte> data, uint32_t stride);

		//----------------------------------------------------------
		/// @brief Selects the indices the next draws read.
		void set_index_buffer(const buffer& indices);

		//----------------------------------------------------------
		/// @brief Sets the per-draw constants (cbuffer register(b0)).
		void set_constants(std::span<const std::byte> constants);

		//----------------------------------------------------------
		/// @brief Sets the texture the pixel shader samples (register(t0)).
		void set_texture(const gpu_texture& sampled_texture);

		//----------------------------------------------------------
		/// @brief Draws triangles straight from the vertex stream.
		/// @param vertex_count Number of vertices (three per triangle).
		/// @param instance_count How many times to draw them; each instance
		/// reads the next element of the per-instance stream.
		void draw(uint32_t vertex_count, uint32_t instance_count = 1);

		//----------------------------------------------------------
		/// @brief Draws triangles from the index buffer.
		/// @param index_count Number of indices (three per triangle).
		/// @param instance_count How many times to draw them; each instance
		/// reads the next element of the per-instance stream.
		void draw_indexed(uint32_t index_count, uint32_t instance_count = 1);

		//----------------------------------------------------------
		/// @brief Returns the pipeline set last, or nullptr before the first.
		/// The 2D and 3D renderers record into the same command list; this
		/// tells one whether the other replaced its pipeline in between.
		const pipeline* get_pipeline() const;

		//----------------------------------------------------------
		/// @brief Returns the backend's data (see gpu_texture::get_native).
		native& get_native();

	private:
		std::unique_ptr<native> m_native; // the backend's data
		const pipeline* m_pipeline;       // set last; not owned
	};
}

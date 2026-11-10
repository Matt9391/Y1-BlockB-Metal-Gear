#include "opengl/backend/command_list.h"

#include "common/constants.h"
#include "core/assert.h"
#include "opengl/backend/buffer.h"
#include "opengl/backend/device.h"
#include "opengl/backend/gpu_texture.h"
#include "opengl/backend/pipeline.h"
#include "opengl/gl_call.h"

#include <optional>
#include <vector>

using namespace buz::rhi::gl;

namespace
{
	//--------------------------------------------------------------
	/// @brief How many floats a vertex format holds.
	GLint get_component_count(buz::rhi::vertex_format format)
	{
		switch (format)
		{
		case buz::rhi::vertex_format::float2: return 2;
		case buz::rhi::vertex_format::float3: return 3;
		case buz::rhi::vertex_format::float4: return 4;
		}
		return 4;
	}
}

namespace buz::rhi
{
	// rhi::command_list hands every call to its native object below. Only
	// this backend's files are compiled (see rhi/CMakeLists.txt).

	//--------------------------------------------------------------
	command_list::command_list(std::unique_ptr<native> data)
		: m_native(std::move(data))
		, m_pipeline(nullptr)
	{
	}

	//--------------------------------------------------------------
	command_list::~command_list() = default;

	//--------------------------------------------------------------
	void command_list::set_pipeline(const pipeline& active_pipeline)
	{
		m_native->set_pipeline(active_pipeline.get_native());
		m_pipeline = &active_pipeline;
	}

	//--------------------------------------------------------------
	void command_list::set_vertex_buffer(const buffer& vertices)
	{
		m_native->set_vertex_buffer(vertices.get_native());
	}

	//--------------------------------------------------------------
	bool command_list::set_vertex_data(vertex_rate rate, std::span<const std::byte> data, uint32_t stride)
	{
		return m_native->set_vertex_data(rate, data, stride);
	}

	//--------------------------------------------------------------
	void command_list::set_index_buffer(const buffer& indices)
	{
		m_native->set_index_buffer(indices.get_native());
	}

	//--------------------------------------------------------------
	void command_list::set_constants(std::span<const std::byte> constants)
	{
		m_native->set_constants(constants);
	}

	//--------------------------------------------------------------
	void command_list::set_texture(const gpu_texture& sampled_texture)
	{
		m_native->set_texture(sampled_texture.get_native());
	}

	//--------------------------------------------------------------
	void command_list::draw(uint32_t vertex_count, uint32_t instance_count)
	{
		m_native->draw(vertex_count, instance_count);
	}

	//--------------------------------------------------------------
	void command_list::draw_indexed(uint32_t index_count, uint32_t instance_count)
	{
		m_native->draw_indexed(index_count, instance_count);
	}

	//--------------------------------------------------------------
	const pipeline* command_list::get_pipeline() const
	{
		return m_pipeline;
	}

	//--------------------------------------------------------------
	command_list::native& command_list::get_native()
	{
		return *m_native;
	}

	//--------------------------------------------------------------
	command_list::native::native()
		: m_owner(nullptr)
		, m_pipeline(nullptr)
		, m_index_type(GL_UNSIGNED_SHORT)
		, m_enabled_inputs(0)
	{
	}

	//--------------------------------------------------------------
	void command_list::native::begin(device::native& owner)
	{
		m_owner = &owner;
		m_pipeline = nullptr;
		m_stats = {};
	}

	//--------------------------------------------------------------
	void command_list::native::set_pipeline(const pipeline::native& active_pipeline)
	{
		m_pipeline = &active_pipeline;
		const pipeline_desc& desc = active_pipeline.get_desc();

		BUZ_GL_CALL(glUseProgram(active_pipeline.get_program()));
		BUZ_GL_CALL(glBindSampler(0, m_owner->get_sampler(desc.sampler_filter)));

		// Standard alpha blending, so drawing with transparency works
		if (desc.blending)
		{
			BUZ_GL_CALL(glEnable(GL_BLEND));
			BUZ_GL_CALL(glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
		}
		else
		{
			BUZ_GL_CALL(glDisable(GL_BLEND));
		}

		// Front faces wind counter clockwise on screen (the glTF convention,
		// and OpenGL's own)
		if (desc.culling == cull_mode::back)
		{
			BUZ_GL_CALL(glEnable(GL_CULL_FACE));
			BUZ_GL_CALL(glFrontFace(GL_CCW));
			BUZ_GL_CALL(glCullFace(GL_BACK));
		}
		else
		{
			BUZ_GL_CALL(glDisable(GL_CULL_FACE));
		}

		// With depth testing, a pixel is only drawn when nothing nearer was
		// drawn there before; 2D drawing goes without, in drawing order.
		if (desc.depth)
		{
			BUZ_GL_CALL(glEnable(GL_DEPTH_TEST));
			BUZ_GL_CALL(glDepthFunc(GL_LEQUAL));
			BUZ_GL_CALL(glDepthMask(GL_TRUE));
		}
		else
		{
			BUZ_GL_CALL(glDisable(GL_DEPTH_TEST));
			BUZ_GL_CALL(glDepthMask(GL_FALSE));
		}

#if !defined(__EMSCRIPTEN__)
		BUZ_GL_CALL(glPolygonMode(GL_FRONT_AND_BACK, desc.fill == fill_mode::wireframe ? GL_LINE : GL_FILL));
#endif
	}

	//--------------------------------------------------------------
	void command_list::native::set_vertex_buffer(const buffer::native& vertices)
	{
		BUZ_ASSERT(vertices.get_usage() == buffer_usage::vertex);
		m_streams[0] = { vertices.get_buffer(), 0, vertices.get_stride() };
	}

	//--------------------------------------------------------------
	bool command_list::native::set_vertex_data(vertex_rate rate, std::span<const std::byte> data, uint32_t stride)
	{
		const std::optional<uintptr_t> offset = m_owner->write_scratch(data, k_frame_scratch_alignment);
		if (!offset)
		{
			return false;
		}

		// Stream 0 holds per-vertex data, stream 1 per-instance data
		m_streams[rate == vertex_rate::per_instance ? 1 : 0] = { m_owner->get_scratch_buffer(), *offset, stride };
		return true;
	}

	//--------------------------------------------------------------
	void command_list::native::set_index_buffer(const buffer::native& indices)
	{
		BUZ_ASSERT(indices.get_usage() == buffer_usage::index16 || indices.get_usage() == buffer_usage::index32);
		BUZ_GL_CALL(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indices.get_buffer()));
		m_index_type = indices.get_usage() == buffer_usage::index32 ? GL_UNSIGNED_INT : GL_UNSIGNED_SHORT;
	}

	//--------------------------------------------------------------
	void command_list::native::set_constants(std::span<const std::byte> constants)
	{
		// The constants go into the scratch buffer too; uniform block 0
		// (buz_constants in the shaders) reads them from there.
		const std::optional<uintptr_t> offset = m_owner->write_scratch(constants, m_owner->get_constants_alignment());
		if (offset)
		{
			BUZ_GL_CALL(glBindBufferRange(GL_UNIFORM_BUFFER, 0, m_owner->get_scratch_buffer(), static_cast<GLintptr>(*offset), static_cast<GLsizeiptr>(constants.size())));
		}
	}

	//--------------------------------------------------------------
	void command_list::native::set_texture(const gpu_texture::native& sampled_texture)
	{
		// Texture unit 0: buz_texture in the shaders
		BUZ_GL_CALL(glBindTexture(GL_TEXTURE_2D, sampled_texture.get_texture()));
	}

	//--------------------------------------------------------------
	void command_list::native::bind_vertex_inputs()
	{
		BUZ_ASSERT_MSG(m_pipeline != nullptr, "set_pipeline must come before the first draw");

		// Input i of the vertex shader is attribute i of the layout
		const std::vector<vertex_attribute>& layout = m_pipeline->get_desc().vertex_layout;
		for (GLuint input = 0; input < layout.size(); ++input)
		{
			const vertex_attribute& attribute = layout[input];
			const GLuint stream_index = attribute.rate == vertex_rate::per_instance ? 1 : 0;
			const vertex_stream& stream = m_streams[stream_index];

			BUZ_GL_CALL(glBindBuffer(GL_ARRAY_BUFFER, stream.buffer));
			const void* offset = reinterpret_cast<const void*>(stream.offset + attribute.offset);
			BUZ_GL_CALL(glVertexAttribPointer(input, get_component_count(attribute.format), GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stream.stride), offset));

			// A divisor of 1 moves on to the next element once per instance
			BUZ_GL_CALL(glVertexAttribDivisor(input, stream_index));
			BUZ_GL_CALL(glEnableVertexAttribArray(input));
		}

		for (GLuint input = static_cast<GLuint>(layout.size()); input < m_enabled_inputs; ++input)
		{
			BUZ_GL_CALL(glDisableVertexAttribArray(input));
		}
		m_enabled_inputs = static_cast<GLuint>(layout.size());
	}

	//--------------------------------------------------------------
	void command_list::native::draw(uint32_t vertex_count, uint32_t instance_count)
	{
		bind_vertex_inputs();
		const GLenum mode = m_pipeline->get_desc().primitive_topology == topology::lines ? GL_LINES : GL_TRIANGLES;
		BUZ_GL_CALL(glDrawArraysInstanced(mode, 0, static_cast<GLsizei>(vertex_count), static_cast<GLsizei>(instance_count)));
		m_stats.draw_calls += 1;
		m_stats.instances += instance_count;
	}

	//--------------------------------------------------------------
	void command_list::native::draw_indexed(uint32_t index_count, uint32_t instance_count)
	{
		bind_vertex_inputs();
		const GLenum mode = m_pipeline->get_desc().primitive_topology == topology::lines ? GL_LINES : GL_TRIANGLES;
		BUZ_GL_CALL(glDrawElementsInstanced(mode, static_cast<GLsizei>(index_count), m_index_type, nullptr, static_cast<GLsizei>(instance_count)));
		m_stats.draw_calls += 1;
		m_stats.instances += instance_count;
	}

	//--------------------------------------------------------------
	const frame_stats& command_list::native::get_stats() const
	{
		return m_stats;
	}
}

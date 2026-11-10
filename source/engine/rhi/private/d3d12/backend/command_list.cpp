#include "d3d12/backend/command_list.h"

#include "core/assert.h"
#include "d3d12/backend/buffer.h"
#include "d3d12/backend/device.h"
#include "d3d12/backend/gpu_texture.h"
#include "d3d12/backend/pipeline.h"
#include "d3d12/descriptor_heap.h"

#include <cstring>
#include <optional>

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
		, m_command_list(nullptr)
	{
	}

	//--------------------------------------------------------------
	void command_list::native::begin(device::native& owner, ID3D12GraphicsCommandList* d3d_command_list)
	{
		m_owner = &owner;
		m_command_list = d3d_command_list;
		m_stats = {};
	}

	//--------------------------------------------------------------
	void command_list::native::set_pipeline(const pipeline::native& active_pipeline)
	{
		m_command_list->SetGraphicsRootSignature(active_pipeline.get_root_signature());
		m_command_list->SetPipelineState(active_pipeline.get_state());
		m_command_list->IASetPrimitiveTopology(active_pipeline.get_topology());
	}

	//--------------------------------------------------------------
	void command_list::native::set_vertex_buffer(const buffer::native& vertices)
	{
		BUZ_ASSERT(vertices.get_usage() == buffer_usage::vertex);
		m_command_list->IASetVertexBuffers(0, 1, &vertices.get_vertex_view());
	}

	//--------------------------------------------------------------
	bool command_list::native::set_vertex_data(vertex_rate rate, std::span<const std::byte> data, uint32_t stride)
	{
		const std::optional<device::native::scratch_allocation> scratch = m_owner->allocate_scratch(static_cast<uint32_t>(data.size()));
		if (!scratch)
		{
			return false;
		}

		std::memcpy(scratch->data, data.data(), data.size());

		// Stream 0 holds per-vertex data, stream 1 per-instance data
		const D3D12_VERTEX_BUFFER_VIEW view = { scratch->gpu_address, static_cast<UINT>(data.size()), stride };
		m_command_list->IASetVertexBuffers(rate == vertex_rate::per_instance ? 1 : 0, 1, &view);
		return true;
	}

	//--------------------------------------------------------------
	void command_list::native::set_index_buffer(const buffer::native& indices)
	{
		BUZ_ASSERT(indices.get_usage() == buffer_usage::index16 || indices.get_usage() == buffer_usage::index32);
		m_command_list->IASetIndexBuffer(&indices.get_index_view());
	}

	//--------------------------------------------------------------
	void command_list::native::set_constants(std::span<const std::byte> constants)
	{
		// Root parameter 0: the constants, as 32 bit values
		m_command_list->SetGraphicsRoot32BitConstants(0, static_cast<UINT>(constants.size() / 4), constants.data(), 0);
	}

	//--------------------------------------------------------------
	void command_list::native::set_texture(const gpu_texture::native& sampled_texture)
	{
		// Root parameter 1: the descriptor table pointing at the texture
		m_command_list->SetGraphicsRootDescriptorTable(1, m_owner->get_texture_heap().get_gpu_handle(sampled_texture.get_descriptor_index()));
	}

	//--------------------------------------------------------------
	void command_list::native::draw(uint32_t vertex_count, uint32_t instance_count)
	{
		m_command_list->DrawInstanced(vertex_count, instance_count, 0, 0);
		m_stats.draw_calls += 1;
		m_stats.instances += instance_count;
	}

	//--------------------------------------------------------------
	void command_list::native::draw_indexed(uint32_t index_count, uint32_t instance_count)
	{
		m_command_list->DrawIndexedInstanced(index_count, instance_count, 0, 0, 0);
		m_stats.draw_calls += 1;
		m_stats.instances += instance_count;
	}

	//--------------------------------------------------------------
	const frame_stats& command_list::native::get_stats() const
	{
		return m_stats;
	}
}

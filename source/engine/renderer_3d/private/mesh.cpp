#include "gpu_mesh.h"

#include "core/log.h"
#include "engine/engine.h"
#include "renderer_3d.h"
#include "rhi/buffer.h"
#include "rhi/device.h"
#include "rhi/render_context.h"

namespace buz
{
	//--------------------------------------------------------------
	mesh::mesh(uint32_t index_count, const aabb& bounds)
		: m_index_count(index_count)
		, m_bounds(bounds)
	{
	}

	//--------------------------------------------------------------
	mesh::~mesh() = default;

	//--------------------------------------------------------------
	const aabb& mesh::get_bounds() const
	{
		return m_bounds;
	}

	//--------------------------------------------------------------
	uint32_t mesh::get_index_count() const
	{
		return m_index_count;
	}

	//--------------------------------------------------------------
	gpu_mesh::gpu_mesh(std::unique_ptr<rhi::buffer> vertices, std::unique_ptr<rhi::buffer> indices, uint32_t index_count, const aabb& bounds)
		: mesh(index_count, bounds)
		, m_vertices(std::move(vertices))
		, m_indices(std::move(indices))
	{
	}

	//--------------------------------------------------------------
	// Defined here: destroying the buffers needs the full rhi::buffer type
	gpu_mesh::~gpu_mesh() = default;

	//--------------------------------------------------------------
	std::shared_ptr<mesh> upload_mesh(const render_context& context, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices)
	{
		if (vertices.empty() || indices.empty())
		{
			log::error("create_mesh: a mesh needs vertices and indices");
			return nullptr;
		}

		aabb bounds = { vertices[0].position, vertices[0].position };
		for (const mesh_vertex& vertex : vertices)
		{
			bounds.min = glm::min(bounds.min, vertex.position);
			bounds.max = glm::max(bounds.max, vertex.position);
		}

		std::unique_ptr<rhi::buffer> vertex_buffer = context.device->create_buffer(rhi::buffer_usage::vertex, std::as_bytes(vertices), sizeof(mesh_vertex));
		std::unique_ptr<rhi::buffer> index_buffer = context.device->create_buffer(rhi::buffer_usage::index32, std::as_bytes(indices), sizeof(uint32_t));
		if (vertex_buffer == nullptr || index_buffer == nullptr)
		{
			return nullptr;
		}

		const uint32_t index_count = static_cast<uint32_t>(indices.size());
		return std::make_shared<gpu_mesh>(std::move(vertex_buffer), std::move(index_buffer), index_count, bounds);
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices)
	{
		return upload_mesh(engine.get_render_context(), vertices, indices);
	}

	//--------------------------------------------------------------
	const rhi::buffer& gpu_mesh::get_vertex_buffer() const
	{
		return *m_vertices;
	}

	//--------------------------------------------------------------
	const rhi::buffer& gpu_mesh::get_index_buffer() const
	{
		return *m_indices;
	}

	//--------------------------------------------------------------
	const gpu_mesh& to_gpu_mesh(const mesh& shape)
	{
		// The engine makes no other kind of mesh
		return static_cast<const gpu_mesh&>(shape);
	}
}

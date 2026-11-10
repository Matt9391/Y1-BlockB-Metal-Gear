#pragma once

#include "renderer_3d/mesh.h"

#include <memory>

namespace buz
{
	namespace rhi
	{
		class buffer;
	}

	//--------------------------------------------------------------
	/// @brief The engine's side of a mesh: the gpu buffers draw_mesh draws.
	/// Every buz::mesh a game holds is one of these.
	class gpu_mesh final : public mesh
	{
	public:
		gpu_mesh(std::unique_ptr<rhi::buffer> vertices, std::unique_ptr<rhi::buffer> indices, uint32_t index_count, const aabb& bounds);
		~gpu_mesh();

		//----------------------------------------------------------
		/// @brief Returns the vertices in gpu memory.
		const rhi::buffer& get_vertex_buffer() const;

		//----------------------------------------------------------
		/// @brief Returns the indices in gpu memory.
		const rhi::buffer& get_index_buffer() const;

	private:
		std::unique_ptr<rhi::buffer> m_vertices;
		std::unique_ptr<rhi::buffer> m_indices;
	};

	//--------------------------------------------------------------
	/// @brief Returns the gpu side of a mesh a game handed to draw_mesh.
	const gpu_mesh& to_gpu_mesh(const mesh& shape);
}

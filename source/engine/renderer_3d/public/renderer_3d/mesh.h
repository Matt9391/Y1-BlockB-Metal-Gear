/// @file
/// @brief Meshes: triangles in gpu memory. Make one with buz::create_mesh
/// or buz::load_mesh from asset_management/model_loading.h.

#pragma once

#include "core/aabb.h"
#include "core/math.h"

#include <cstdint>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief One corner of a triangle of a mesh.
	struct mesh_vertex
	{
		/// @brief Where the corner is.
		vec3 position = { 0.0f, 0.0f, 0.0f };
		/// @brief The direction the surface faces, for lighting.
		vec3 normal = { 0.0f, 1.0f, 0.0f };
		/// @brief Where on the texture this corner is, 0..1.
		vec2 uv = { 0.0f, 0.0f };
	};

	//--------------------------------------------------------------
	/// @brief Triangles in gpu memory, ready to be drawn with draw_mesh.
	///
	/// Create one with create_mesh from vertices and indices, or load one
	/// with load_mesh (both in asset_management/model_loading.h). Like
	/// textures, meshes are handed out as a std::shared_ptr: the mesh lives
	/// as long as anything holds it.
	class mesh
	{
	public:
		mesh(const mesh&) = delete;
		mesh& operator=(const mesh&) = delete;

		//----------------------------------------------------------
		/// @brief Returns the box around every vertex, in the mesh's own space.
		const aabb& get_bounds() const;

		//----------------------------------------------------------
		/// @brief Returns how many indices (three per triangle) the mesh has.
		uint32_t get_index_count() const;

	protected:
		// Only the engine makes meshes (gpu_mesh, its side of this class,
		// holds the gpu buffers)
		mesh(uint32_t index_count, const aabb& bounds);
		~mesh();

	private:
		uint32_t m_index_count; // three per triangle
		aabb m_bounds;          // around every vertex, in the mesh's own space
	};
}

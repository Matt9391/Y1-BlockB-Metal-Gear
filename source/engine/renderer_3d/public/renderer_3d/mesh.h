/// @file
/// @brief Meshes: triangles in gpu memory, and making them from code.

#pragma once

#include "core/aabb.h"
#include "core/math.h"

#include <cstdint>
#include <memory>
#include <span>

namespace buz
{
	class engine;

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
	/// with load_mesh (renderer_3d/model.h). Like textures, meshes are handed out
	/// as a std::shared_ptr: the mesh lives as long as anything holds it.
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

	//--------------------------------------------------------------
	/// @brief Uploads triangles made in code to the gpu.
	/// @param engine The engine, which owns the graphics card.
	/// @param vertices The corners.
	/// @param indices Three per triangle, counter clockwise seen from the front.
	/// @return The mesh, or nullptr when uploading failed.
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);
}

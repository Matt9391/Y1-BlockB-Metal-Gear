#pragma once

#include "core/mesh_data.h"
#include "renderer_3d/mesh.h"

#include <cstdint>
#include <memory>
#include <span>

namespace buz
{
	struct render_context;

	//--------------------------------------------------------------
	/// @brief Uploads triangles to the gpu. For engine modules: a game
	/// calls the version in asset_management/model_loading.h, which takes
	/// the engine.
	/// @param context The engine's render context
	/// (engine::get_render_context), which owns the graphics card.
	/// @param vertices The corners.
	/// @param indices Three per triangle, counter clockwise seen from the front.
	/// @return The mesh, or nullptr when uploading failed (the reason is logged).
	std::shared_ptr<mesh> create_mesh(const render_context& context, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);
}

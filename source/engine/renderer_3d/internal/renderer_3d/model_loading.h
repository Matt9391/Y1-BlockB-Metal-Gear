#pragma once

#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

namespace buz
{
	struct render_context;

	//--------------------------------------------------------------
	/// @brief Loads a glTF model (.gltf or .glb) from the content folder.
	/// For engine modules: a game calls the version in
	/// asset_management/model_loading.h, which takes the engine.
	///
	/// Every call loads the file again and uploads new meshes.
	/// @param context The engine's render context
	/// (engine::get_render_context); its graphics card gets the meshes.
	/// @param content_relative_path Model file, relative to the content folder.
	/// @return The model, or nothing when it could not be loaded (the reason is logged).
	std::optional<model> load_model(const render_context& context, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Loads every mesh of a glTF file as one mesh, placed as in the
	/// file. For engine modules: a game calls the version in
	/// asset_management/model_loading.h.
	/// @param context The engine's render context
	/// (engine::get_render_context); its graphics card gets the mesh.
	/// @param content_relative_path Model file, relative to the content folder.
	/// @return The mesh, or nullptr when it could not be loaded.
	std::shared_ptr<mesh> load_mesh(const render_context& context, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Uploads triangles made in code to the gpu. For engine modules:
	/// a game calls the version in asset_management/model_loading.h.
	/// @param context The engine's render context
	/// (engine::get_render_context), which owns the graphics card.
	/// @param vertices The corners.
	/// @param indices Three per triangle, counter clockwise seen from the front.
	/// @return The mesh, or nullptr when uploading failed.
	std::shared_ptr<mesh> create_mesh(const render_context& context, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);
}

/// @file
/// @brief Loading models and meshes from the content folder, and making meshes from triangles.

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
	class engine;

	//--------------------------------------------------------------
	/// @brief Loads a glTF model (.gltf or .glb) from the content folder,
	/// with its parts, their hierarchy and its animations:
	/// @code{.cpp}
	/// std::optional<buz::model> soldier = buz::load_model(engine, "models/soldier.glb");
	/// @endcode
	///
	/// Every call loads the file again and uploads new meshes: loading a
	/// model once and reusing it is up to you.
	/// @param engine The engine, which owns the graphics card.
	/// @param content_relative_path Model file, relative to the content folder, with / between folders.
	/// @return The model, or nothing when it could not be loaded (the reason is logged).
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Loads every mesh of a glTF file as one mesh, placed as in the
	/// file: for props and level geometry that never move apart.
	/// @param engine The engine, which owns the graphics card.
	/// @param content_relative_path Model file, relative to the content folder, with / between folders.
	/// @return The mesh, or nullptr when it could not be loaded (the reason is logged).
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path);

	//--------------------------------------------------------------
	/// @brief Makes a mesh from triangles made in code.
	/// @param engine The engine, which owns the graphics card.
	/// @param vertices The corners.
	/// @param indices Three per triangle, counter clockwise seen from the front.
	/// @return The mesh, or nullptr when uploading failed.
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);
}

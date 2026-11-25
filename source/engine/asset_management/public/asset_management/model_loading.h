/// @file
/// @brief Loading models, meshes and their textures from the content folder, and making meshes from triangles.

#pragma once

#include "core/mesh_data.h"
#include "core/texture_id.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"

#include <cstdint>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include <yes_no.h>

namespace buz
{
	class engine;
	class texture;

	//--------------------------------------------------------------
	/// @brief Whether vertex normals that point against their triangles are
	/// turned around (see model_import_options::normals).
	using orient_normals = utl::yes_no_t<struct orient_normals_tag>;

	//--------------------------------------------------------------
	/// @brief Whether texture coordinates outside 0..1 are moved into it
	/// (see model_import_options::uvs).
	using wrap_uvs = utl::yes_no_t<struct wrap_uvs_tag>;

	//--------------------------------------------------------------
	/// @brief Repairs for models that look right in the viewer they came from
	/// but not in the engine, typically files saved out of a web viewer
	/// (three.js and the like), which draws both sides of every triangle and
	/// repeats textures. Everything is off by default: a model made in
	/// Blender needs none of it.
	/// @code{.cpp}
	/// buz::load_model(engine, "my_game/models/soldier.glb", { .normals = buz::orient_normals::yes, .uvs = buz::wrap_uvs::yes });
	/// @endcode
	struct model_import_options
	{
		/// @brief Turns around every vertex normal that points against the
		/// triangles it belongs to. The triangles' winding (counter clockwise
		/// seen from the front) is taken as the truth, so what culling keeps
		/// and how it is lit agree. Fixes a model that draws the right faces
		/// but is lit as if from behind (dark, light from the wrong side).
		/// It does not fix a model whose winding itself is reversed (the
		/// outside culled away): that needs fixing in Blender.
		orient_normals normals = orient_normals::no;

		/// @brief Moves each primitive's texture coordinates by whole numbers
		/// so they start in 0..1. The engine clamps textures at their edges,
		/// so coordinates in, say, -1..0 all read the edge pixels (streaks);
		/// a viewer that repeats textures showed the same texels this picks.
		wrap_uvs uvs = wrap_uvs::no;
	};

	//--------------------------------------------------------------
	/// @brief Reads a glTF model (.gltf or .glb) from the content folder
	/// onto the cpu: the nodes with their triangles and materials, and the
	/// animations. Nothing goes to the graphics card, so it needs no engine.
	/// @param content_relative_path Model file, relative to the content folder, with / between folders.
	/// @param options Repairs to apply while reading; none by default.
	/// @return The model, or nothing when it could not be read (the reason is logged).
	std::optional<model_data> import_model(std::string_view content_relative_path, const model_import_options& options = {});

	//--------------------------------------------------------------
	/// @brief Bakes every node's triangles into one mesh_data, placed as
	/// in the file (what load_mesh uploads).
	mesh_data merge_meshes(const model_data& source);

	//--------------------------------------------------------------
	/// @brief Loads a glTF model (.gltf or .glb) from the content folder,
	/// with its parts, their hierarchy and its animations:
	/// @code{.cpp}
	/// std::optional<buz::model> soldier = buz::load_model(engine, "my_game/models/soldier.glb");
	/// @endcode
	///
	/// Every call loads the file again and uploads new meshes: loading a
	/// model once and reusing it is up to you. Textures are not loaded: each
	/// node's material names its image (material::base_texture), and
	/// load_texture loads it.
	/// @param engine The engine, which owns the graphics card.
	/// @param content_relative_path Model file, relative to the content folder, with / between folders.
	/// @param options Repairs to apply while reading; none by default.
	/// @return The model, or nothing when it could not be loaded (the reason is logged).
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path, const model_import_options& options = {});

	//--------------------------------------------------------------
	/// @brief Loads every mesh of a glTF file as one mesh, placed as in the
	/// file: for props and level geometry that never move apart.
	/// @param engine The engine, which owns the graphics card.
	/// @param content_relative_path Model file, relative to the content folder, with / between folders.
	/// @param options Repairs to apply while reading; none by default.
	/// @return The mesh, or nullptr when it could not be loaded (the reason is logged).
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path, const model_import_options& options = {});

	//--------------------------------------------------------------
	/// @brief Makes a mesh from triangles made in code.
	/// @param engine The engine, which owns the graphics card.
	/// @param vertices The corners.
	/// @param indices Three per triangle, counter clockwise seen from the front.
	/// @return The mesh, or nullptr when uploading failed.
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);

	//--------------------------------------------------------------
	/// @brief Loads the image a material names, an image file or one inside
	/// a model file:
	/// @code{.cpp}
	/// std::shared_ptr<buz::texture> skin = buz::load_texture(engine, soldier.nodes[0].look.base_texture);
	/// @endcode
	/// Every call loads the image again. Like the other load_texture, a
	/// failed load is logged and returns the magenta placeholder.
	/// @param engine The engine, which owns the graphics card.
	/// @param id The image, from a material.
	/// @return The uploaded texture, or the placeholder when loading failed.
	std::shared_ptr<texture> load_texture(engine& engine, const texture_id& id);
}

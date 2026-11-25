#include "asset_management/model_loading.h"

#include "core/log.h"
#include "core/paths.h"
#include "core/texture_source.h"
#include "engine/engine.h"
#include "render_core/render_context.h"
#include "render_core/texture_loader.h"
#include "renderer_3d/mesh_creation.h"

#include <vector>

namespace
{
	//--------------------------------------------------------------
	/// @brief Returns where every node sits in the model as placed in the
	/// file: a node is placed relative to its parent, so its placement is
	/// its parent's times its own. Parents come before their children.
	std::vector<buz::mat4> compute_rest_placements(const buz::model_data& source)
	{
		std::vector<buz::mat4> placements;
		placements.reserve(source.nodes.size());
		for (const buz::model_node_data& node : source.nodes)
		{
			const buz::node_transform& transform = node.local_transform;
			const buz::mat4 local = glm::translate(buz::mat4(1.0f), transform.translation) * glm::mat4_cast(transform.rotation) * glm::scale(buz::mat4(1.0f), transform.scale);
			placements.push_back(node.parent >= 0 ? placements[static_cast<size_t>(node.parent)] * local : local);
		}
		return placements;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	mesh_data merge_meshes(const model_data& source)
	{
		const std::vector<mat4> world = compute_rest_placements(source);

		mesh_data merged;
		size_t total_vertices = 0;
		size_t total_indices = 0;
		for (const model_node_data& node : source.nodes)
		{
			total_vertices += node.triangles.vertices.size();
			total_indices += node.triangles.indices.size();
		}
		merged.vertices.reserve(total_vertices);
		merged.indices.reserve(total_indices);

		for (size_t index = 0; index < source.nodes.size(); ++index)
		{
			const mesh_data& part = source.nodes[index].triangles;
			const mat3 normal_matrix = transpose(inverse(mat3(world[index])));
			const uint32_t first_vertex = static_cast<uint32_t>(merged.vertices.size());

			for (mesh_vertex vertex : part.vertices)
			{
				vertex.position = vec3(world[index] * vec4(vertex.position, 1.0f));
				vertex.normal = normalize(normal_matrix * vertex.normal);
				merged.vertices.push_back(vertex);
			}
			for (const uint32_t vertex_index : part.indices)
			{
				merged.indices.push_back(first_vertex + vertex_index);
			}
		}
		return merged;
	}

	//--------------------------------------------------------------
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path, const model_import_options& options)
	{
		const std::optional<model_data> file = import_model(content_relative_path, options);
		if (!file)
		{
			return std::nullopt;
		}

		model result;
		result.clips = file->clips;
		result.nodes.reserve(file->nodes.size());
		for (const model_node_data& node : file->nodes)
		{
			model_node uploaded;
			uploaded.name = node.name;
			uploaded.parent = node.parent;
			uploaded.local_transform = node.local_transform;
			uploaded.look = node.look;
			if (!node.triangles.indices.empty())
			{
				uploaded.shape = create_mesh(engine, node.triangles.vertices, node.triangles.indices);
			}
			result.nodes.push_back(std::move(uploaded));
		}

		log::info("Loaded model \"{}\" ({} nodes, {} clips)", content_relative_path, result.nodes.size(), result.clips.size());
		return result;
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path, const model_import_options& options)
	{
		const std::optional<model_data> file = import_model(content_relative_path, options);
		if (!file)
		{
			return nullptr;
		}

		const mesh_data merged = merge_meshes(*file);
		if (merged.indices.empty())
		{
			log::error("Model \"{}\" has no triangles", content_relative_path);
			return nullptr;
		}
		return create_mesh(engine, merged.vertices, merged.indices);
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices)
	{
		return create_mesh(engine.get_render_context(), vertices, indices);
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> load_texture(engine& engine, const texture_id& id)
	{
		texture_loader& textures = *engine.get_render_context().textures;
		const texture_source* source = id.get_source();
		if (source == nullptr)
		{
			// Decoding nothing logs the error and hands out the placeholder
			return textures.load_from_memory({}, "a texture_id that names no image");
		}
		if (source->path.empty())
		{
			return textures.load_from_memory(source->embedded_image, id.what());
		}
		return textures.load_from_file(paths::get_content_path(source->path));
	}
}

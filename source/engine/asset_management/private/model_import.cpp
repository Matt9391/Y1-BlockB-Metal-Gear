#include "asset_management/model_loading.h"

#include "core/log.h"
#include "core/paths.h"
#include "texture_id_creation.h"

#include <cgltf.h>

// matrix_decompose is a glm "experimental" extension; it has been stable for years
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <cmath>
#include <format>
#include <fstream>
#include <iterator>
#include <string>
#include <vector>

namespace
{
	// Clips are sampled this many times per second; PS1-era games animated
	// at 15 to 30 frames per second
	constexpr float k_sample_rate = 30.0f;

	//--------------------------------------------------------------
	/// @brief Frees a cgltf document when it goes out of scope.
	struct gltf_document
	{
		cgltf_data* data = nullptr;
		~gltf_document() { cgltf_free(data); }
	};

	//--------------------------------------------------------------
	/// @brief Reads a node's transform from the file: a matrix, or separate
	/// translation, rotation and scale.
	buz::node_transform read_node_transform(const cgltf_node& node)
	{
		buz::node_transform transform;
		if (node.has_matrix)
		{
			const buz::mat4 matrix = glm::make_mat4(node.matrix);
			buz::vec3 skew;
			buz::vec4 perspective;
			glm::decompose(matrix, transform.scale, transform.rotation, transform.translation, skew, perspective);
			return transform;
		}

		if (node.has_translation) { transform.translation = { node.translation[0], node.translation[1], node.translation[2] }; }
		// glTF stores quaternions as x, y, z, w; glm's constructor takes w first
		if (node.has_rotation) { transform.rotation = buz::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]); }
		if (node.has_scale) { transform.scale = { node.scale[0], node.scale[1], node.scale[2] }; }
		return transform;
	}

	//--------------------------------------------------------------
	/// @brief What the repairs of model_import_options changed, for the log.
	struct repair_counts
	{
		size_t normals_turned = 0;
		size_t uv_sets_moved = 0;
	};

	//--------------------------------------------------------------
	/// @brief Turns around the normals of vertices first_vertex onwards that
	/// point against their triangles (indices first_index onwards). A
	/// vertex's facing is the sum of its triangles' counter clockwise face
	/// normals; their length is twice the area, so big triangles weigh more.
	void orient_primitive_normals(buz::mesh_data& mesh, size_t first_vertex, size_t first_index, repair_counts& counts)
	{
		std::vector<buz::vec3> facing(mesh.vertices.size() - first_vertex, buz::vec3(0.0f));
		for (size_t i = first_index; i + 2 < mesh.indices.size(); i += 3)
		{
			const uint32_t a = mesh.indices[i];
			const uint32_t b = mesh.indices[i + 1];
			const uint32_t c = mesh.indices[i + 2];
			const buz::vec3 face = glm::cross(mesh.vertices[b].position - mesh.vertices[a].position, mesh.vertices[c].position - mesh.vertices[a].position);
			facing[a - first_vertex] += face;
			facing[b - first_vertex] += face;
			facing[c - first_vertex] += face;
		}
		for (size_t v = first_vertex; v < mesh.vertices.size(); ++v)
		{
			if (glm::dot(mesh.vertices[v].normal, facing[v - first_vertex]) < 0.0f)
			{
				mesh.vertices[v].normal = -mesh.vertices[v].normal;
				++counts.normals_turned;
			}
		}
	}

	//--------------------------------------------------------------
	/// @brief Moves the uvs of vertices first_vertex onwards by whole numbers,
	/// per axis, so the smallest lands in 0..1.
	void wrap_primitive_uvs(buz::mesh_data& mesh, size_t first_vertex, repair_counts& counts)
	{
		if (first_vertex >= mesh.vertices.size())
		{
			return;
		}
		buz::vec2 lowest = mesh.vertices[first_vertex].uv;
		for (size_t v = first_vertex; v < mesh.vertices.size(); ++v)
		{
			lowest = glm::min(lowest, mesh.vertices[v].uv);
		}
		// The small margin keeps a coordinate of exactly 0 (stored as -0.0000001) where it is
		const buz::vec2 offset = -glm::floor(lowest + buz::vec2(1e-6f));
		if (offset == buz::vec2(0.0f))
		{
			return;
		}
		for (size_t v = first_vertex; v < mesh.vertices.size(); ++v)
		{
			mesh.vertices[v].uv += offset;
		}
		++counts.uv_sets_moved;
	}

	//--------------------------------------------------------------
	/// @brief Collects every triangle primitive of a glTF mesh into one list,
	/// repairing each as the options ask.
	buz::mesh_data read_mesh(const cgltf_mesh& source, const buz::model_import_options& options, repair_counts& counts)
	{
		// Count the vertices and indices of every triangle primitive first,
		// so each list is allocated once
		size_t total_vertices = 0;
		size_t total_indices = 0;
		for (cgltf_size p = 0; p < source.primitives_count; ++p)
		{
			const cgltf_primitive& primitive = source.primitives[p];
			const cgltf_accessor* positions = cgltf_find_accessor(&primitive, cgltf_attribute_type_position, 0);
			if (primitive.type == cgltf_primitive_type_triangles && positions != nullptr)
			{
				total_vertices += positions->count;
				total_indices += primitive.indices != nullptr ? primitive.indices->count : positions->count;
			}
		}

		buz::mesh_data result;
		result.vertices.reserve(total_vertices);
		result.indices.reserve(total_indices);
		for (cgltf_size p = 0; p < source.primitives_count; ++p)
		{
			const cgltf_primitive& primitive = source.primitives[p];
			if (primitive.type != cgltf_primitive_type_triangles)
			{
				continue;
			}

			const cgltf_accessor* positions = nullptr;
			const cgltf_accessor* normals = nullptr;
			const cgltf_accessor* uvs = nullptr;
			for (cgltf_size a = 0; a < primitive.attributes_count; ++a)
			{
				const cgltf_attribute& attribute = primitive.attributes[a];
				if (attribute.type == cgltf_attribute_type_position) { positions = attribute.data; }
				if (attribute.type == cgltf_attribute_type_normal) { normals = attribute.data; }
				if (attribute.type == cgltf_attribute_type_texcoord && attribute.index == 0) { uvs = attribute.data; }
			}
			if (positions == nullptr)
			{
				continue;
			}

			const uint32_t first_vertex = static_cast<uint32_t>(result.vertices.size());
			const size_t first_index = result.indices.size();
			for (cgltf_size v = 0; v < positions->count; ++v)
			{
				buz::mesh_vertex vertex;
				cgltf_accessor_read_float(positions, v, &vertex.position.x, 3);
				if (normals != nullptr) { cgltf_accessor_read_float(normals, v, &vertex.normal.x, 3); }
				if (uvs != nullptr) { cgltf_accessor_read_float(uvs, v, &vertex.uv.x, 2); }
				result.vertices.push_back(vertex);
			}

			// Unindexed primitives use every vertex once, in order
			const cgltf_size index_count = primitive.indices != nullptr ? primitive.indices->count : positions->count;
			for (cgltf_size i = 0; i < index_count; ++i)
			{
				const cgltf_size index = primitive.indices != nullptr ? cgltf_accessor_read_index(primitive.indices, i) : i;
				result.indices.push_back(first_vertex + static_cast<uint32_t>(index));
			}

			// Normals that point inward only make sense with the normals the file has
			if (options.normals && normals != nullptr)
			{
				orient_primitive_normals(result, first_vertex, first_index, counts);
			}
			if (options.uvs && uvs != nullptr)
			{
				wrap_primitive_uvs(result, first_vertex, counts);
			}
		}
		return result;
	}

	//--------------------------------------------------------------
	/// @brief Names the image a material's texture shows. Every image of a
	/// file gets its id once (image_ids, by cgltf_image_index), so materials
	/// that share an image share the id.
	buz::texture_id read_image_id(const cgltf_data& data, const cgltf_image& image, std::string_view model_path, std::vector<buz::texture_id>& image_ids)
	{
		buz::texture_id& id = image_ids[cgltf_image_index(&data, &image)];
		if (id.has_image())
		{
			return id;
		}

		// An image is either bytes inside the model's own buffers (always
		// the case in a .glb), or a file next to the model
		if (image.buffer_view != nullptr)
		{
			const cgltf_buffer_view& view = *image.buffer_view;
			const uint8_t* bytes = static_cast<const uint8_t*>(view.buffer->data) + view.offset;
			const std::string name = image.name != nullptr ? std::format(" \"{}\"", image.name) : "";
			const std::string description = std::format("{}, image {}{}", model_path, cgltf_image_index(&data, &image), name);
			id = buz::make_embedded_texture_id(description, std::vector<uint8_t>(bytes, bytes + view.size));
		}
		else if (image.uri != nullptr)
		{
			const buz::fs::path directory = buz::fs::path(std::u8string(model_path.begin(), model_path.end())).parent_path();
			const std::u8string relative = (directory / image.uri).lexically_normal().generic_u8string();
			id = buz::texture_id(std::string_view(reinterpret_cast<const char*>(relative.data()), relative.size()));
		}
		return id;
	}

	//--------------------------------------------------------------
	/// @brief Reads a material's color, (at most one) texture, lighting and alpha cutoff.
	buz::material read_material(const cgltf_data& data, const cgltf_material* source, std::string_view model_path, std::vector<buz::texture_id>& image_ids)
	{
		buz::material look;
		if (source == nullptr)
		{
			return look;
		}

		const cgltf_pbr_metallic_roughness& pbr = source->pbr_metallic_roughness;
		look.base_color = { pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3] };
		look.lit = source->unlit ? buz::lighting::no : buz::lighting::yes;
		// Alpha mode MASK cuts out the pixels below the file's cutoff (0.5
		// unless the artist changed it); OPAQUE and BLEND draw every pixel
		if (source->alpha_mode == cgltf_alpha_mode_mask)
		{
			look.alpha_cutoff = source->alpha_cutoff;
		}

		const cgltf_texture* texture = pbr.base_color_texture.texture;
		if (texture != nullptr && texture->image != nullptr)
		{
			look.base_texture = read_image_id(data, *texture->image, model_path, image_ids);
		}
		return look;
	}

	//--------------------------------------------------------------
	/// @brief Reads a value of an animation sampler's output at a key.
	/// Cubic spline outputs store in-tangent, value, out-tangent per key;
	/// only the value is used (a straight line between keys).
	void read_key(const cgltf_animation_sampler& sampler, cgltf_size key, float* out, int components)
	{
		const cgltf_size element = sampler.interpolation == cgltf_interpolation_type_cubic_spline ? key * 3 + 1 : key;
		cgltf_accessor_read_float(sampler.output, element, out, components);
	}

	//--------------------------------------------------------------
	/// @brief Applies one animation channel to one node's transform.
	void apply_channel(const cgltf_animation_channel& channel, float time, buz::node_transform& transform)
	{
		const cgltf_animation_sampler& sampler = *channel.sampler;
		const cgltf_size key_count = sampler.input->count;
		if (key_count == 0)
		{
			return;
		}

		// Find the keys around the time; before the first or after the last
		// key the nearest key holds
		cgltf_size next = 0;
		float next_time = 0.0f;
		while (next < key_count)
		{
			cgltf_accessor_read_float(sampler.input, next, &next_time, 1);
			if (next_time >= time) { break; }
			++next;
		}

		cgltf_size previous = next == 0 ? 0 : next - 1;
		next = std::min(next, key_count - 1);

		float previous_time = 0.0f;
		cgltf_accessor_read_float(sampler.input, previous, &previous_time, 1);
		cgltf_accessor_read_float(sampler.input, next, &next_time, 1);

		float blend = next_time > previous_time ? (time - previous_time) / (next_time - previous_time) : 0.0f;
		blend = std::clamp(blend, 0.0f, 1.0f);
		if (sampler.interpolation == cgltf_interpolation_type_step) { blend = 0.0f; }

		if (channel.target_path == cgltf_animation_path_type_rotation)
		{
			float a[4] = {};
			float b[4] = {};
			read_key(sampler, previous, a, 4);
			read_key(sampler, next, b, 4);
			transform.rotation = glm::slerp(buz::quat(a[3], a[0], a[1], a[2]), buz::quat(b[3], b[0], b[1], b[2]), blend);
		}
		else if (channel.target_path == cgltf_animation_path_type_translation || channel.target_path == cgltf_animation_path_type_scale)
		{
			buz::vec3 a;
			buz::vec3 b;
			read_key(sampler, previous, &a.x, 3);
			read_key(sampler, next, &b.x, 3);
			(channel.target_path == cgltf_animation_path_type_translation ? transform.translation : transform.scale) = glm::mix(a, b, blend);
		}
	}

}

namespace buz
{
	//--------------------------------------------------------------
	std::optional<model_data> import_model(std::string_view content_relative_path, const model_import_options& options)
	{
		const fs::path path = paths::get_content_path(content_relative_path);

		gltf_document document;
		const cgltf_options gltf_options = {};
		if (cgltf_parse_file(&gltf_options, path.string().c_str(), &document.data) != cgltf_result_success || cgltf_load_buffers(&gltf_options, document.data, path.string().c_str()) != cgltf_result_success)
		{
			log::error("Could not load model \"{}\"", path.string());
			return std::nullopt;
		}

		const cgltf_data& data = *document.data;
		model_data result;
		// Where each node of the file ended up in the model (cgltf_node_index),
		// -1 for a node the scene does not use
		std::vector<int> node_indices(data.nodes_count, -1);
		std::vector<texture_id> image_ids(data.images_count);
		repair_counts repaired;
		// The scene lists each node at most once
		result.nodes.reserve(data.nodes_count);

		// Walk the scene from its roots, so every parent is listed before
		// its children; walking the hierarchy relies on that order.
		std::vector<std::pair<const cgltf_node*, int>> pending;
		pending.reserve(data.nodes_count);
		const cgltf_scene* scene = data.scene != nullptr ? data.scene : (data.scenes_count > 0 ? &data.scenes[0] : nullptr);
		if (scene != nullptr)
		{
			for (cgltf_size i = scene->nodes_count; i > 0; --i)
			{
				pending.push_back({ scene->nodes[i - 1], -1 });
			}
		}

		while (!pending.empty())
		{
			const auto [node, parent] = pending.back();
			pending.pop_back();

			const int index = static_cast<int>(result.nodes.size());
			node_indices[cgltf_node_index(&data, node)] = index;

			model_node_data loaded;
			loaded.name = node->name != nullptr ? node->name : "";
			loaded.parent = parent;
			loaded.local_transform = read_node_transform(*node);
			if (node->mesh != nullptr)
			{
				loaded.triangles = read_mesh(*node->mesh, options, repaired);
			}
			if (node->mesh != nullptr && node->mesh->primitives_count > 0)
			{
				const cgltf_material* first_material = node->mesh->primitives[0].material;
				loaded.look = read_material(data, first_material, content_relative_path, image_ids);
			}
			result.nodes.push_back(std::move(loaded));

			for (cgltf_size i = node->children_count; i > 0; --i)
			{
				pending.push_back({ node->children[i - 1], index });
			}
		}

		if (options.normals || options.uvs)
		{
			log::info("Repaired model \"{}\": {} normals turned around, {} uv sets moved into 0..1", content_relative_path, repaired.normals_turned, repaired.uv_sets_moved);
		}

		// Sample every animation into a pose (a transform per node) per frame
		std::vector<node_transform> rest_pose;
		rest_pose.reserve(result.nodes.size());
		for (const model_node_data& node : result.nodes)
		{
			rest_pose.push_back(node.local_transform);
		}
		result.clips.reserve(data.animations_count);
		for (cgltf_size a = 0; a < data.animations_count; ++a)
		{
			const cgltf_animation& animation = data.animations[a];

			float duration = 0.0f;
			for (cgltf_size s = 0; s < animation.samplers_count; ++s)
			{
				float last = 0.0f;
				cgltf_accessor_read_float(animation.samplers[s].input, animation.samplers[s].input->count - 1, &last, 1);
				duration = std::max(duration, last);
			}

			keyframe_clip clip;
			clip.name = animation.name != nullptr ? animation.name : "animation " + std::to_string(a);
			clip.frames_per_second = k_sample_rate;

			const int frame_count = static_cast<int>(std::floor(duration * k_sample_rate)) + 1;
			clip.frames.reserve(static_cast<size_t>(frame_count));
			for (int frame = 0; frame < frame_count; ++frame)
			{
				std::vector<node_transform> poses = rest_pose;
				const float time = static_cast<float>(frame) / k_sample_rate;
				for (cgltf_size c = 0; c < animation.channels_count; ++c)
				{
					const cgltf_animation_channel& channel = animation.channels[c];
					const int target = channel.target_node != nullptr ? node_indices[cgltf_node_index(&data, channel.target_node)] : -1;
					if (target >= 0)
					{
						apply_channel(channel, time, poses[static_cast<size_t>(target)]);
					}
				}
				clip.frames.push_back(std::move(poses));
			}
			result.clips.push_back(std::move(clip));
		}

		return result;
	}
}

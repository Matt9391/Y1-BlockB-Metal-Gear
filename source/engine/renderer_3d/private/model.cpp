#include "renderer_3d/model.h"

#include "core/log.h"
#include "core/paths.h"
#include "engine/engine.h"
#include "renderer_3d/mesh.h"
#include "rhi/render_context.h"
#include "rhi/texture_loader.h"

#include <cgltf.h>

// matrix_decompose is a glm "experimental" extension; it has been stable for years
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

namespace
{
	// Clips are sampled at this rate; the PS1 era animated at 15-30 frames
	constexpr float k_sample_rate = 30.0f;

	//--------------------------------------------------------------
	/// @brief The textures of a file, one slot per image of the file
	/// (cgltf_image_index): a texture several materials share is loaded once.
	using image_textures = std::vector<std::shared_ptr<buz::texture>>;

	//--------------------------------------------------------------
	/// @brief A node's triangles on the cpu, before they go to the gpu.
	struct cpu_mesh
	{
		std::vector<buz::mesh_vertex> vertices;
		std::vector<uint32_t> indices;
	};

	//--------------------------------------------------------------
	/// @brief Frees a cgltf document when it goes out of scope.
	struct gltf_document
	{
		cgltf_data* data = nullptr;
		~gltf_document() { cgltf_free(data); }
	};

	//--------------------------------------------------------------
	/// @brief Reads a node's pose from the file: a matrix, or separate
	/// translation, rotation and scale.
	buz::joint_pose read_pose(const cgltf_node& node)
	{
		buz::joint_pose pose;
		if (node.has_matrix)
		{
			const buz::mat4 matrix = glm::make_mat4(node.matrix);
			buz::vec3 skew;
			buz::vec4 perspective;
			glm::decompose(matrix, pose.scale, pose.rotation, pose.translation, skew, perspective);
			return pose;
		}

		if (node.has_translation) { pose.translation = { node.translation[0], node.translation[1], node.translation[2] }; }
		// glTF stores quaternions as x, y, z, w; glm's constructor takes w first
		if (node.has_rotation) { pose.rotation = buz::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]); }
		if (node.has_scale) { pose.scale = { node.scale[0], node.scale[1], node.scale[2] }; }
		return pose;
	}

	//--------------------------------------------------------------
	/// @brief Collects every triangle primitive of a glTF mesh into one list.
	cpu_mesh read_mesh(const cgltf_mesh& source)
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

		cpu_mesh result;
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
		}
		return result;
	}

	//--------------------------------------------------------------
	/// @brief Loads a material's color and (at most one) texture.
	buz::material read_material(const cgltf_data& data, const cgltf_material* source, buz::texture_loader& textures, const buz::fs::path& directory, image_textures& loaded_images)
	{
		buz::material look;
		if (source == nullptr)
		{
			return look;
		}

		const cgltf_pbr_metallic_roughness& pbr = source->pbr_metallic_roughness;
		look.base_color = { pbr.base_color_factor[0], pbr.base_color_factor[1], pbr.base_color_factor[2], pbr.base_color_factor[3] };
		look.lit = source->unlit ? buz::lighting::no : buz::lighting::yes;

		const cgltf_texture* texture = pbr.base_color_texture.texture;
		if (texture == nullptr || texture->image == nullptr)
		{
			return look;
		}

		const cgltf_image* image = texture->image;
		std::shared_ptr<buz::texture>& loaded = loaded_images[cgltf_image_index(&data, image)];
		if (loaded != nullptr)
		{
			look.base_texture = loaded;
			return look;
		}

		// An image is either a file next to the model, or bytes inside the
		// model's own buffers (always the case in a .glb)
		if (image->buffer_view != nullptr)
		{
			const cgltf_buffer_view& view = *image->buffer_view;
			const uint8_t* bytes = static_cast<const uint8_t*>(view.buffer->data) + view.offset;
			look.base_texture = textures.load_from_memory({ bytes, view.size }, image->name != nullptr ? image->name : "embedded image");
		}
		else if (image->uri != nullptr)
		{
			look.base_texture = textures.load_from_file(directory / image->uri);
		}

		loaded = look.base_texture;
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
	/// @brief Applies one animation channel at a time to a pose.
	void apply_channel(const cgltf_animation_channel& channel, float time, buz::joint_pose& pose)
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
			pose.rotation = glm::slerp(buz::quat(a[3], a[0], a[1], a[2]), buz::quat(b[3], b[0], b[1], b[2]), blend);
		}
		else if (channel.target_path == cgltf_animation_path_type_translation || channel.target_path == cgltf_animation_path_type_scale)
		{
			buz::vec3 a;
			buz::vec3 b;
			read_key(sampler, previous, &a.x, 3);
			read_key(sampler, next, &b.x, 3);
			(channel.target_path == cgltf_animation_path_type_translation ? pose.translation : pose.scale) = glm::mix(a, b, blend);
		}
	}

	//--------------------------------------------------------------
	/// @brief Everything read from a file, still on the cpu.
	struct loaded_file
	{
		buz::model model;             // the nodes and clips; meshes not uploaded yet
		std::vector<cpu_mesh> meshes; // per node, empty for empty nodes
	};

	//--------------------------------------------------------------
	/// @brief Parses a glTF file into nodes, cpu meshes and sampled clips.
	std::optional<loaded_file> read_file(buz::engine& engine, std::string_view content_relative_path)
	{
		const buz::fs::path path = buz::paths::get_content_path(content_relative_path);

		gltf_document document;
		const cgltf_options options = {};
		if (cgltf_parse_file(&options, path.string().c_str(), &document.data) != cgltf_result_success || cgltf_load_buffers(&options, document.data, path.string().c_str()) != cgltf_result_success)
		{
			buz::log::error("Could not load model \"{}\"", path.string());
			return std::nullopt;
		}

		const cgltf_data& data = *document.data;
		loaded_file result;
		// Where each node of the file ended up in the model (cgltf_node_index),
		// -1 for a node the scene does not use
		std::vector<int> node_indices(data.nodes_count, -1);
		image_textures loaded_images(data.images_count);
		// The scene lists each node at most once
		result.meshes.reserve(data.nodes_count);
		result.model.nodes.reserve(data.nodes_count);

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

			const int index = static_cast<int>(result.model.nodes.size());
			node_indices[cgltf_node_index(&data, node)] = index;

			buz::model_node loaded;
			loaded.name = node->name != nullptr ? node->name : "";
			loaded.parent = parent;
			loaded.local_pose = read_pose(*node);
			result.meshes.push_back(node->mesh != nullptr ? read_mesh(*node->mesh) : cpu_mesh{});
			if (node->mesh != nullptr && node->mesh->primitives_count > 0)
			{
				const cgltf_material* first_material = node->mesh->primitives[0].material;
				buz::texture_loader& textures = *engine.get_render_context().textures;
				const buz::fs::path directory = path.parent_path();
				loaded.look = read_material(data, first_material, textures, directory, loaded_images);
			}
			result.model.nodes.push_back(std::move(loaded));

			for (cgltf_size i = node->children_count; i > 0; --i)
			{
				pending.push_back({ node->children[i - 1], index });
			}
		}

		// Sample every animation into a pose per node per frame
		const std::vector<buz::joint_pose> rest_pose = result.model.get_rest_pose();
		result.model.clips.reserve(data.animations_count);
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

			buz::keyframe_clip clip;
			clip.name = animation.name != nullptr ? animation.name : "animation " + std::to_string(a);
			clip.frames_per_second = k_sample_rate;

			const int frame_count = static_cast<int>(std::floor(duration * k_sample_rate)) + 1;
			clip.frames.reserve(static_cast<size_t>(frame_count));
			for (int frame = 0; frame < frame_count; ++frame)
			{
				std::vector<buz::joint_pose> poses = rest_pose;
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
			result.model.clips.push_back(std::move(clip));
		}

		return result;
	}
}

namespace
{
	//--------------------------------------------------------------
	/// @brief Returns where every node sits in the model as placed in the
	/// file: a node is placed relative to its parent, so its placement is
	/// its parent's times its own. Parents come before their children.
	std::vector<buz::mat4> compute_rest_placements(const buz::model& source)
	{
		std::vector<buz::mat4> placements;
		placements.reserve(source.nodes.size());
		for (const buz::model_node& node : source.nodes)
		{
			const buz::joint_pose& pose = node.local_pose;
			const buz::mat4 local = glm::translate(buz::mat4(1.0f), pose.translation) * glm::mat4_cast(pose.rotation) * glm::scale(buz::mat4(1.0f), pose.scale);
			placements.push_back(node.parent >= 0 ? placements[static_cast<size_t>(node.parent)] * local : local);
		}
		return placements;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	int model::find_node(const std::string& name) const
	{
		for (size_t index = 0; index < nodes.size(); ++index)
		{
			if (nodes[index].name == name) { return static_cast<int>(index); }
		}
		return -1;
	}

	//--------------------------------------------------------------
	const keyframe_clip* model::find_clip(const std::string& name) const
	{
		for (const keyframe_clip& clip : clips)
		{
			if (clip.name == name) { return &clip; }
		}
		return nullptr;
	}

	//--------------------------------------------------------------
	std::vector<joint_pose> model::get_rest_pose() const
	{
		std::vector<joint_pose> poses;
		poses.reserve(nodes.size());
		for (const model_node& node : nodes)
		{
			poses.push_back(node.local_pose);
		}
		return poses;
	}

	//--------------------------------------------------------------
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path)
	{
		std::optional<loaded_file> file = read_file(engine, content_relative_path);
		if (!file)
		{
			return std::nullopt;
		}

		for (size_t index = 0; index < file->model.nodes.size(); ++index)
		{
			const cpu_mesh& triangles = file->meshes[index];
			if (!triangles.indices.empty())
			{
				file->model.nodes[index].shape = create_mesh(engine, triangles.vertices, triangles.indices);
			}
		}

		log::info("Loaded model \"{}\" ({} nodes, {} clips)", content_relative_path, file->model.nodes.size(), file->model.clips.size());
		return std::move(file->model);
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path)
	{
		const std::optional<loaded_file> file = read_file(engine, content_relative_path);
		if (!file)
		{
			return nullptr;
		}

		// Bake every node's placement into its vertices and merge them
		const std::vector<mat4> world = compute_rest_placements(file->model);

		cpu_mesh merged;
		size_t total_vertices = 0;
		size_t total_indices = 0;
		for (const cpu_mesh& part : file->meshes)
		{
			total_vertices += part.vertices.size();
			total_indices += part.indices.size();
		}
		merged.vertices.reserve(total_vertices);
		merged.indices.reserve(total_indices);

		for (size_t index = 0; index < file->meshes.size(); ++index)
		{
			const cpu_mesh& part = file->meshes[index];
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

		if (merged.indices.empty())
		{
			log::error("Model \"{}\" has no triangles", content_relative_path);
			return nullptr;
		}

		return create_mesh(engine, merged.vertices, merged.indices);
	}
}

#include "renderer_3d.h"

#include "gpu_mesh.h"

#include "core/assert.h"
#include "core/log.h"
#include "render_core/sampler_filter.h"
#include "render_core/shader_library.h"
#include "render_core/texture_loader.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/mesh_creation.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"
#include "rhi/pipeline.h"

#include <cstddef>

namespace
{
	//--------------------------------------------------------------
	/// @brief Constants sent to the mesh shaders for every draw (must match
	/// diffuse_vs.hlsl and diffuse_ps.hlsl): exactly the 128 bytes every backend
	/// guarantees. The camera is in frame_data.
	struct mesh_constants
	{
		buz::mat4 world; // object space to world space

		// Rows of the normal matrix. Shaders read constants in rows of four
		// floats, so each row shares its four floats with one setting.
		buz::vec3 normal_row_0;
		float lit;          // 1 when lit, 0 when not
		buz::vec3 normal_row_1;
		float alpha_cutoff; // material::alpha_cutoff
		buz::vec3 normal_row_2;
		float padding;      // fills the last row

		buz::color base_color; // the material color
	};

	static_assert(sizeof(mesh_constants) <= buz::rhi::k_max_constants_size, "mesh constants no longer fit the per-draw constants");
	static_assert(offsetof(mesh_constants, base_color) == 112, "every normal row and its setting must fill 16 bytes, as the shaders read them");

	//--------------------------------------------------------------
	/// @brief The camera, sent to the mesh shaders once a frame and again
	/// when it changes (register(b2); must match diffuse_vs.hlsl and
	/// diffuse_ps.hlsl).
	struct frame_data
	{
		buz::mat4 view_projection; // world space to the screen
		buz::vec4 camera_position; // in world space; w is 1 and fills the row
	};

	static_assert(sizeof(frame_data) <= buz::rhi::k_max_frame_data_size, "the frame data no longer fits");
	static_assert(offsetof(frame_data, camera_position) == 64, "the camera position must start a row of four floats, as the shaders read it");

	//--------------------------------------------------------------
	/// @brief The two matrices of a camera (see world_renderer_3d::set_camera).
	struct camera_matrices
	{
		buz::mat4 view;       // world space to camera space
		buz::mat4 projection; // camera space to the screen
	};

	//--------------------------------------------------------------
	/// @brief The camera a world is seen through until the game sets one:
	/// up, behind and to the side of the origin, looking at it.
	camera_matrices make_default_camera(const buz::rhi::device& device)
	{
		const float aspect_ratio = static_cast<float>(device.get_width()) / static_cast<float>(device.get_height());
		const buz::mat4 view = glm::lookAtRH(buz::vec3(8.0f, 10.0f, 13.0f), buz::vec3(0.0f, 0.0f, 0.0f), buz::vec3(0.0f, 1.0f, 0.0f));
		const buz::mat4 projection = glm::perspectiveRH_ZO(buz::radians(60.0f), aspect_ratio, 0.1f, 200.0f);
		return { view, projection };
	}

	//--------------------------------------------------------------
	/// @brief Constants of the debug line shaders.
	struct line_constants
	{
		buz::mat4 view_projection;
	};
}

namespace buz
{
	//--------------------------------------------------------------
	renderer_3d::renderer_3d(const render_context& context)
		: m_context(context)
		, m_commands(nullptr)
		, m_view_projection(1.0f)
		, m_camera_position(0.0f)
		, m_camera_uploaded(false)
		, m_shader_data_uploaded(false)
	{
		const camera_matrices default_camera = make_default_camera(*context.device);
		set_camera(default_camera.view, default_camera.projection);

		shader_library& shaders = *context.shaders;
		set_surface_shading(surface_shading{});

		rhi::pipeline_desc line_desc;
		line_desc.name = "debug_line_pipeline";
		line_desc.vertex_shader = shaders.get_shader("line_vs");
		line_desc.pixel_shader = shaders.get_shader("line_ps");
		line_desc.vertex_layout = {
			{ .semantic = "POSITION", .format = rhi::vertex_format::float3, .offset = offsetof(line_vertex, position) },
			{ .semantic = "COLOR", .format = rhi::vertex_format::float4, .offset = offsetof(line_vertex, line_color) },
		};
		line_desc.constants_size = sizeof(line_constants);
		line_desc.primitive_topology = rhi::topology::lines;
		line_desc.depth = depth_test::yes;
		m_line_pipeline = context.device->create_pipeline(line_desc);

		for (size_t shape = 0; shape < m_primitives.size(); ++shape)
		{
			const mesh_data data = make_primitive(static_cast<primitive>(shape));
			m_primitives[shape] = create_mesh(context, data.vertices, data.indices);
		}

		const uint8_t white_pixel[4] = { 255, 255, 255, 255 };
		m_white_texture = context.textures->create_from_pixels(1, 1, white_pixel);

		log::info("3D renderer ready");
	}

	//--------------------------------------------------------------
	renderer_3d::~renderer_3d() = default;

	//--------------------------------------------------------------
	void renderer_3d::begin_frame(rhi::command_list& commands)
	{
		m_commands = &commands;

		// A new frame selects no shader data and no camera until a mesh
		// draw copies them in
		m_shader_data_uploaded = false;
		m_camera_uploaded = false;
	}

	//--------------------------------------------------------------
	void renderer_3d::end_frame()
	{
		m_commands = nullptr;
	}

	//--------------------------------------------------------------
	void renderer_3d::set_debug_views(const debug_views& views)
	{
		m_debug_views = views;
	}

	//--------------------------------------------------------------
	const debug_views& renderer_3d::get_debug_views() const
	{
		return m_debug_views;
	}

	//--------------------------------------------------------------
	void renderer_3d::set_surface_shading(const surface_shading& shading)
	{
		// Building pipelines is slow, so it only happens when the shading
		// changes
		if (m_mesh_pipelines[0] != nullptr && shading == m_surface_shading)
		{
			return;
		}
		m_surface_shading = shading;

		// The same shaders and layout in two pipelines: solid, and wireframe
		// (a debug view that changes fixed state)
		rhi::pipeline_desc desc;
		desc.vertex_shader = m_context.shaders->get_shader(shading.vertex_shader);
		desc.pixel_shader = m_context.shaders->get_shader(shading.pixel_shader);
		desc.vertex_layout = {
			{ .semantic = "POSITION", .format = rhi::vertex_format::float3, .offset = offsetof(mesh_vertex, position) },
			{ .semantic = "NORMAL", .format = rhi::vertex_format::float3, .offset = offsetof(mesh_vertex, normal) },
			{ .semantic = "TEXCOORD", .format = rhi::vertex_format::float2, .offset = offsetof(mesh_vertex, uv) },
		};
		desc.constants_size = sizeof(mesh_constants);
		desc.depth = depth_test::yes;
		desc.filter = to_sampler_filter(shading.filter);

		for (int wireframe = 0; wireframe < 2; ++wireframe)
		{
			desc.name = shading.vertex_shader + (wireframe ? "_wireframe" : "");
			desc.fill = wireframe ? rhi::fill_mode::wireframe : rhi::fill_mode::solid;
			desc.culling = wireframe ? rhi::cull_mode::none : rhi::cull_mode::back;
			m_mesh_pipelines[wireframe] = m_context.device->create_pipeline(desc);
		}
	}

	//--------------------------------------------------------------
	void renderer_3d::set_shader_data(std::span<const std::byte> data)
	{
		BUZ_ASSERT_MSG(data.size() <= rhi::k_max_shader_data_size, "shader data must be at most {} bytes, not {}", rhi::k_max_shader_data_size, data.size());
		m_shader_data.assign(data.begin(), data.end());
		m_shader_data_uploaded = false;
	}

	//--------------------------------------------------------------
	void renderer_3d::upload_shader_data(rhi::command_list& commands)
	{
		// The command list keeps it selected for the rest of the frame,
		// whatever pipelines the 2D side sets in between
		if (m_shader_data.empty() || m_shader_data_uploaded)
		{
			return;
		}

		m_shader_data_uploaded = commands.set_shader_data(m_shader_data);
	}

	//--------------------------------------------------------------
	void renderer_3d::set_camera(const mat4& view, const mat4& projection)
	{
		// Column-vector convention: the view applies first, then the
		// projection, so the projection comes first in the product. The
		// view moves the camera to the origin, so its inverse moves the
		// origin to the camera.
		m_view_projection = projection * view;
		m_camera_position = vec3(inverse(view)[3]);
		m_camera_uploaded = false;
	}

	//--------------------------------------------------------------
	void renderer_3d::upload_frame_data(rhi::command_list& commands)
	{
		// Like the shader data, it stays selected for the rest of the frame
		if (m_camera_uploaded)
		{
			return;
		}

		const frame_data camera = { m_view_projection, vec4(m_camera_position, 1.0f) };
		m_camera_uploaded = commands.set_frame_data(std::as_bytes(std::span(&camera, 1)));
	}

	//--------------------------------------------------------------
	rhi::command_list& renderer_3d::get_commands() const
	{
		// The frame's command list is only set while the game draws the
		// world (game_3d::on_render_world)
		BUZ_ASSERT_MSG(m_commands != nullptr, "the 3D world can only be drawn while on_render_world runs");
		return *m_commands;
	}

	//--------------------------------------------------------------
	const rhi::pipeline& renderer_3d::select_mesh_pipeline() const
	{
		return *m_mesh_pipelines[m_debug_views.wireframe ? 1 : 0];
	}

	//--------------------------------------------------------------
	void renderer_3d::draw(const mesh& shape, const material& look, const texture* surface, const mat4& transform)
	{
		// The sprites' pipeline is replaced below; the next sprite draw sees
		// the change (rhi::command_list::get_pipeline) and sets it again
		rhi::command_list& commands = get_commands();
		commands.set_pipeline(select_mesh_pipeline());
		upload_shader_data(commands);
		upload_frame_data(commands);
		const gpu_mesh& buffers = to_gpu_mesh(shape);
		commands.set_vertex_buffer(buffers.get_vertex_buffer());
		commands.set_index_buffer(buffers.get_index_buffer());

		// Normals turn with the object, but must not stretch with a non
		// uniform scale: the inverse transpose of the world matrix does both.
		// The rows of that transpose are the columns of the inverse, and glm
		// stores a matrix as its columns.
		const mat3 inverse_world = inverse(mat3(transform));
		const bool lit = look.lit && m_debug_views.lighting;

		mesh_constants constants;
		constants.world = transform;
		constants.normal_row_0 = inverse_world[0];
		constants.lit = lit ? 1.0f : 0.0f;
		constants.normal_row_1 = inverse_world[1];
		constants.alpha_cutoff = look.alpha_cutoff;
		constants.normal_row_2 = inverse_world[2];
		constants.padding = 0.0f;
		constants.base_color = look.base_color;

		commands.set_constants(std::as_bytes(std::span(&constants, 1)));
		commands.set_texture((surface != nullptr ? *surface : *m_white_texture).get_gpu_texture());
		commands.draw_indexed(shape.get_index_count());
	}

	//--------------------------------------------------------------
	void renderer_3d::draw(primitive shape, const material& look, const mat4& transform)
	{
		draw(*m_primitives[static_cast<size_t>(shape)], look, nullptr, transform);
	}

	//--------------------------------------------------------------
	void renderer_3d::draw_lines(std::span<const line_vertex> vertices)
	{
		if (!m_debug_views.debug_lines || vertices.empty())
		{
			return;
		}

		rhi::command_list& commands = get_commands();
		commands.set_pipeline(*m_line_pipeline);
		if (!commands.set_vertex_data(rhi::vertex_rate::per_vertex, std::as_bytes(vertices), sizeof(line_vertex)))
		{
			return;
		}

		const line_constants constants = { m_view_projection };
		commands.set_constants(std::as_bytes(std::span(&constants, 1)));
		commands.draw(static_cast<uint32_t>(vertices.size()));
	}
}

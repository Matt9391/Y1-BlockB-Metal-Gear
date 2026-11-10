#include "renderer_3d.h"

#include "gpu_mesh.h"

#include "core/assert.h"
#include "core/log.h"
#include "renderer_3d/mesh.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"
#include "rhi/pipeline.h"
#include "rhi/shader_library.h"
#include "rhi/texture_loader.h"

#include <cstddef>

namespace
{
	//--------------------------------------------------------------
	/// @brief Constants sent to the mesh shaders for every draw (must match
	/// mesh_vs.hlsl and mesh_ps.hlsl): exactly the 128 bytes every backend
	/// guarantees.
	struct mesh_constants
	{
		buz::mat4 world_view_projection; // object space to the screen
		buz::vec4 normal_rows[3];        // w of row 0: lit
		buz::color base_color;           // the material color
	};

	static_assert(sizeof(mesh_constants) <= buz::rhi::k_max_constants_size, "mesh constants no longer fit the per-draw constants");

	//--------------------------------------------------------------
	/// @brief The camera a world is seen through until the game sets one:
	/// up, behind and to the side of the origin, looking at it.
	buz::mat4 make_default_camera(const buz::rhi::device& device)
	{
		const float aspect_ratio = static_cast<float>(device.get_width()) / static_cast<float>(device.get_height());
		const buz::mat4 view = glm::lookAtRH(buz::vec3(8.0f, 10.0f, 13.0f), buz::vec3(0.0f, 0.0f, 0.0f), buz::vec3(0.0f, 1.0f, 0.0f));
		const buz::mat4 projection = glm::perspectiveRH_ZO(buz::radians(60.0f), aspect_ratio, 0.1f, 200.0f);
		return projection * view;
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
		, m_view_projection(make_default_camera(*context.device))
		, m_wireframe(false)
		, m_lighting_off(false)
		, m_show_debug(true)
	{
		shader_library& shaders = *context.shaders;
		set_mesh_shading(mesh_shading{});

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
			m_primitives[shape] = upload_mesh(context, data.vertices, data.indices);
		}

		const uint8_t white_pixel[4] = { 255, 255, 255, 255 };
		m_white_texture = context.textures->create_from_pixels(1, 1, white_pixel);

		log::info("3D renderer ready");
	}

	//--------------------------------------------------------------
	renderer_3d::~renderer_3d() = default;

	//--------------------------------------------------------------
	void renderer_3d::set_mesh_shading(const mesh_shading& shading)
	{
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
		desc.sampler_filter = shading.filter;

		for (int wireframe = 0; wireframe < 2; ++wireframe)
		{
			desc.name = shading.vertex_shader + (wireframe ? "_wireframe" : "");
			desc.fill = wireframe ? rhi::fill_mode::wireframe : rhi::fill_mode::solid;
			desc.culling = wireframe ? rhi::cull_mode::none : rhi::cull_mode::back;
			m_mesh_pipelines[wireframe] = m_context.device->create_pipeline(desc);
		}
	}

	//--------------------------------------------------------------
	void renderer_3d::set_view_projection(const mat4& view_projection)
	{
		m_view_projection = view_projection;
	}

	//--------------------------------------------------------------
	void renderer_3d::toggle_wireframe()
	{
		m_wireframe = !m_wireframe;
	}

	//--------------------------------------------------------------
	void renderer_3d::toggle_lighting()
	{
		m_lighting_off = !m_lighting_off;
	}

	//--------------------------------------------------------------
	void renderer_3d::toggle_debug_lines()
	{
		m_show_debug = !m_show_debug;
	}

	//--------------------------------------------------------------
	rhi::command_list& renderer_3d::get_commands() const
	{
		// The world pass only records while game::on_render_world runs
		BUZ_ASSERT(m_context.world_commands != nullptr);
		return *m_context.world_commands;
	}

	//--------------------------------------------------------------
	const rhi::pipeline& renderer_3d::select_mesh_pipeline() const
	{
		return *m_mesh_pipelines[m_wireframe ? 1 : 0];
	}

	//--------------------------------------------------------------
	void renderer_3d::draw(const mesh& shape, const material& look, const mat4& transform)
	{
		// The sprites' pipeline is replaced below; the next sprite notices
		// (rhi::command_list::get_pipeline) and sets it again
		rhi::command_list& commands = get_commands();
		commands.set_pipeline(select_mesh_pipeline());
		const gpu_mesh& buffers = to_gpu_mesh(shape);
		commands.set_vertex_buffer(buffers.get_vertex_buffer());
		commands.set_index_buffer(buffers.get_index_buffer());

		// Normals turn with the object, but must not stretch with a non
		// uniform scale: the inverse transpose of the world matrix does both.
		// glm stores columns, the shader wants rows.
		const mat3 normal_matrix = transpose(inverse(mat3(transform)));
		const bool lit = look.lit && !m_lighting_off;

		mesh_constants constants;
		constants.world_view_projection = m_view_projection * transform;
		for (int row = 0; row < 3; ++row)
		{
			constants.normal_rows[row] = { normal_matrix[0][row], normal_matrix[1][row], normal_matrix[2][row], 0.0f };
		}
		constants.normal_rows[0].w = lit ? 1.0f : 0.0f;
		constants.base_color = look.base_color;

		commands.set_constants(std::as_bytes(std::span(&constants, 1)));
		commands.set_texture(rhi::to_gpu_texture(look.base_texture != nullptr ? *look.base_texture : *m_white_texture));
		commands.draw_indexed(shape.get_index_count());
	}

	//--------------------------------------------------------------
	void renderer_3d::draw(primitive shape, const material& look, const mat4& transform)
	{
		draw(*m_primitives[static_cast<size_t>(shape)], look, transform);
	}

	//--------------------------------------------------------------
	void renderer_3d::draw_lines(std::span<const line_vertex> vertices)
	{
		if (!m_show_debug || vertices.empty())
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

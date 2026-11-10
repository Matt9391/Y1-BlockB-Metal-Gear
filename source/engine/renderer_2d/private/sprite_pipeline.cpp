#include "renderer_2d/sprite_pipeline.h"

#include "rhi/buffer.h"
#include "rhi/command_list.h"
#include "rhi/device.h"
#include "rhi/pipeline.h"
#include "rhi/shader_library.h"
#include "rhi/texture_loader.h"
#include "sprite_layout.h"

#include <cstddef>
#include <cstdint>

namespace buz
{
	//--------------------------------------------------------------
	sprite_pipeline::sprite_pipeline() = default;
	sprite_pipeline::~sprite_pipeline() = default;

	//--------------------------------------------------------------
	bool sprite_pipeline::initialize(rhi::device& graphics_device, shader_library& shaders, texture_loader& textures, texture_filter sampler_filter)
	{
		// The vertex layout must match the sprite_vertex struct and the
		// input of sprite_vs.hlsl.
		rhi::pipeline_desc desc;
		desc.name = "sprite_pipeline";
		desc.vertex_shader = shaders.get_shader("sprite_vs");
		desc.pixel_shader = shaders.get_shader("sprite_ps");
		desc.vertex_layout = {
			{ .semantic = "POSITION", .format = rhi::vertex_format::float2, .offset = offsetof(sprite_vertex, position) },
			{ .semantic = "TEXCOORD", .format = rhi::vertex_format::float2, .offset = offsetof(sprite_vertex, uv) },
		};
		desc.constants_size = sizeof(sprite_constants);
		desc.sampler_filter = sampler_filter;
		desc.blending = alpha_blending::yes; // sprites with transparency

		m_pipeline = graphics_device.create_pipeline(desc);
		if (m_pipeline == nullptr)
		{
			return false;
		}

		// The instanced pipeline reads the same quad per vertex, plus one
		// gpu_sprite_instance per sprite, and shares the pixel shader.
		rhi::pipeline_desc instanced_desc = desc;
		instanced_desc.name = "sprite_instanced_pipeline";
		instanced_desc.vertex_shader = shaders.get_shader("sprite_instanced_vs");
		instanced_desc.vertex_layout.push_back({ .semantic = "TEXCOORD", .semantic_index = 1, .format = rhi::vertex_format::float4, .offset = offsetof(gpu_sprite_instance, position_size), .rate = rhi::vertex_rate::per_instance });
		instanced_desc.vertex_layout.push_back({ .semantic = "TEXCOORD", .semantic_index = 2, .format = rhi::vertex_format::float4, .offset = offsetof(gpu_sprite_instance, uv_rect), .rate = rhi::vertex_rate::per_instance });
		instanced_desc.vertex_layout.push_back({ .semantic = "TEXCOORD", .semantic_index = 3, .format = rhi::vertex_format::float4, .offset = offsetof(gpu_sprite_instance, tint), .rate = rhi::vertex_rate::per_instance });
		instanced_desc.vertex_layout.push_back({ .semantic = "TEXCOORD", .semantic_index = 4, .format = rhi::vertex_format::float4, .offset = offsetof(gpu_sprite_instance, rotation), .rate = rhi::vertex_rate::per_instance });
		instanced_desc.constants_size = sizeof(instanced_constants);

		m_instanced_pipeline = graphics_device.create_pipeline(instanced_desc);
		if (m_instanced_pipeline == nullptr)
		{
			return false;
		}

		// A unit quad centered around the origin; draw_sprite scales,
		// rotates and moves it into place. Uv (0, 0) is the top-left
		// corner of the texture.
		const sprite_vertex vertices[] = {
			{ { -0.5f, -0.5f }, { 0.0f, 0.0f } }, // top-left
			{ { 0.5f, -0.5f }, { 1.0f, 0.0f } },  // top-right
			{ { 0.5f, 0.5f }, { 1.0f, 1.0f } },   // bottom-right
			{ { -0.5f, 0.5f }, { 0.0f, 1.0f } },  // bottom-left
		};

		const uint16_t indices[] = { 0, 1, 2, 0, 2, 3 };

		m_quad_vertices = graphics_device.create_buffer(rhi::buffer_usage::vertex, std::as_bytes(std::span(vertices)), sizeof(sprite_vertex));
		m_quad_indices = graphics_device.create_buffer(rhi::buffer_usage::index16, std::as_bytes(std::span(indices)), sizeof(uint16_t));
		if (m_quad_vertices == nullptr || m_quad_indices == nullptr)
		{
			return false;
		}

		// The built-in white texture behind draw_rect and draw_line
		const uint8_t white_pixel[4] = { 255, 255, 255, 255 };
		m_white_texture = textures.create_from_pixels(1, 1, white_pixel);
		return m_white_texture != nullptr;
	}

	//--------------------------------------------------------------
	void sprite_pipeline::bind(rhi::command_list& commands) const
	{
		commands.set_pipeline(*m_pipeline);
		commands.set_vertex_buffer(*m_quad_vertices);
		commands.set_index_buffer(*m_quad_indices);
	}

	//--------------------------------------------------------------
	void sprite_pipeline::bind_for_shapes(rhi::command_list& commands) const
	{
		commands.set_pipeline(*m_pipeline);
	}

	//--------------------------------------------------------------
	void sprite_pipeline::bind_instanced(rhi::command_list& commands) const
	{
		commands.set_pipeline(*m_instanced_pipeline);
		commands.set_vertex_buffer(*m_quad_vertices);
		commands.set_index_buffer(*m_quad_indices);
	}

	//--------------------------------------------------------------
	const texture& sprite_pipeline::get_white_texture() const
	{
		return *m_white_texture;
	}
}

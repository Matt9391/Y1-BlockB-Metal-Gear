#include "renderer_2d/sprite_pass.h"

#include "renderer_2d/sprite_pipeline.h"
#include "rhi/command_list.h"
#include "rhi/gpu_texture.h"
#include "rhi/pipeline.h"
#include "sprite_layout.h"

#include <cstddef>
#include <cstdint>

namespace
{
	//--------------------------------------------------------------
	/// @brief Builds the world transformation for a sprite quad.
	///
	/// The renderer draws a unit quad from (-0.5, -0.5) to (0.5, 0.5). This
	/// transform scales it to the requested size, rotates it around its
	/// center and then moves it so its center sits at the given position.
	/// @param position Position of the sprite center in pixels.
	/// @param size Width and height of the sprite in pixels.
	/// @param rotation_radians Clockwise rotation around the sprite center.
	buz::mat4 make_sprite_transform(buz::vec2 position, buz::vec2 size, float rotation_radians)
	{
		// Column-vector convention: the transform written last is applied
		// first, so the quad is scaled, then rotated, then translated.
		buz::mat4 transform = glm::translate(buz::mat4(1.0f), buz::vec3(position, 0.0f));
		transform = glm::rotate(transform, rotation_radians, buz::vec3(0.0f, 0.0f, 1.0f));
		transform = glm::scale(transform, buz::vec3(size, 1.0f));

		return transform;
	}
}

namespace buz
{
	//--------------------------------------------------------------
	sprite_pass::sprite_pass()
		: m_commands(nullptr)
		, m_bound(bound_state::nothing)
		, m_bound_pipeline(nullptr)
		, m_pipeline(nullptr)
		, m_view_projection(1.0f)
	{
	}

	//--------------------------------------------------------------
	void sprite_pass::begin(rhi::command_list& commands, const sprite_pipeline& pipeline, const mat4& view_projection)
	{
		m_commands = &commands;
		m_pipeline = &pipeline;
		m_view_projection = view_projection;

		// Nothing is bound yet: the first draw sets up what it needs
		m_bound = bound_state::nothing;
	}

	//--------------------------------------------------------------
	void sprite_pass::bind(bound_state state)
	{
		// The 3D renderer draws into the same command list: when it set a
		// pipeline of its own in between, this pass sets up its state again
		if (m_bound == state && m_commands->get_pipeline() == m_bound_pipeline)
		{
			return;
		}

		switch (state)
		{
		case bound_state::sprite_quad: m_pipeline->bind(*m_commands); break;
		case bound_state::sprite_shapes: m_pipeline->bind_for_shapes(*m_commands); break;
		case bound_state::instanced: m_pipeline->bind_instanced(*m_commands); break;
		case bound_state::nothing: break;
		}

		m_bound = state;
		m_bound_pipeline = m_commands->get_pipeline();
	}

	//--------------------------------------------------------------
	void sprite_pass::submit_quad(const texture& quad_texture, vec4 uv_rect, vec2 position, vec2 size, float rotation_radians, color tint)
	{
		// Combine the quad transformation with the camera (column-vector
		// convention: the view projection is applied last, so it comes
		// first in the multiplication).
		sprite_constants constants;
		constants.transform = m_view_projection * make_sprite_transform(position, size, rotation_radians);
		constants.tint = tint;
		constants.uv_rect = uv_rect;

		bind(bound_state::sprite_quad);
		m_commands->set_constants(std::as_bytes(std::span(&constants, 1)));
		m_commands->set_texture(rhi::to_gpu_texture(quad_texture));
		m_commands->draw_indexed(6);
	}

	//--------------------------------------------------------------
	void sprite_pass::submit_triangles(std::span<const vec2> corners, color fill_color)
	{
		// The sprite shader draws these too: with the white texture and the
		// camera as the only transformation, a vertex lands exactly where
		// its corner says, in the fill color.
		std::vector<sprite_vertex> vertices;
		vertices.reserve(corners.size());
		for (const vec2& corner : corners)
		{
			vertices.push_back({ corner, { 0.0f, 0.0f } });
		}

		bind(bound_state::sprite_shapes);

		if (!m_commands->set_vertex_data(rhi::vertex_rate::per_vertex, std::as_bytes(std::span(vertices)), sizeof(sprite_vertex)))
		{
			return;
		}

		sprite_constants constants;
		constants.transform = m_view_projection;
		constants.tint = fill_color;
		constants.uv_rect = { 0.0f, 0.0f, 1.0f, 1.0f };

		m_commands->set_constants(std::as_bytes(std::span(&constants, 1)));
		m_commands->set_texture(rhi::to_gpu_texture(m_pipeline->get_white_texture()));
		m_commands->draw(static_cast<uint32_t>(vertices.size()));
	}

	//--------------------------------------------------------------
	void sprite_pass::submit_instances(const texture& sprite_texture, std::span<const sprite_instance> instances)
	{
		if (instances.empty())
		{
			return;
		}

		// Convert every sprite into what the shader reads: source rects in
		// texture pixels become 0..1 texture space here, once per sprite.
		const vec2 texture_size = sprite_texture.get_size();
		m_instance_data.clear();
		for (const sprite_instance& instance : instances)
		{
			const bool whole_texture = instance.source_rect.size() == vec2(0.0f, 0.0f);
			const vec2 uv_offset = whole_texture ? vec2(0.0f, 0.0f) : instance.source_rect.min / texture_size;
			const vec2 uv_scale = whole_texture ? vec2(1.0f, 1.0f) : instance.source_rect.size() / texture_size;

			m_instance_data.push_back({ instance.position.x, instance.position.y, instance.size.x, instance.size.y });
			m_instance_data.push_back({ uv_offset.x, uv_offset.y, uv_scale.x, uv_scale.y });
			m_instance_data.push_back({ instance.tint.r, instance.tint.g, instance.tint.b, instance.tint.a });
			m_instance_data.push_back({ instance.rotation_radians, 0.0f, 0.0f, 0.0f });
		}

		bind(bound_state::instanced);

		if (!m_commands->set_vertex_data(rhi::vertex_rate::per_instance, std::as_bytes(std::span(m_instance_data)), sizeof(gpu_sprite_instance)))
		{
			return;
		}

		const instanced_constants constants = { m_view_projection };
		m_commands->set_constants(std::as_bytes(std::span(&constants, 1)));
		m_commands->set_texture(rhi::to_gpu_texture(sprite_texture));
		m_commands->draw_indexed(6, static_cast<uint32_t>(instances.size()));
	}

	//--------------------------------------------------------------
	const texture& sprite_pass::get_white_texture() const
	{
		return m_pipeline->get_white_texture();
	}

	//--------------------------------------------------------------
	void sprite_pass::set_view_projection(const mat4& view_projection)
	{
		m_view_projection = view_projection;
	}
}

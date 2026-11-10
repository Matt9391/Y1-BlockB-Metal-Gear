#pragma once

#include "core/math.h"
#include "renderer_2d/renderer_2d.h"

#include <span>
#include <vector>

namespace buz
{
	namespace rhi
	{
		class command_list;
		class pipeline;
	}

	class sprite_pipeline;
	class texture;

	//--------------------------------------------------------------
	/// @brief The engine's side of renderer_2d: the gpu work of one pass
	/// (the world or the screen). It records quads, triangles and instances
	/// into the frame's command list, and only switches the pipeline when
	/// the next draw needs another one. Games draw through world_renderer_2d
	/// and screen_renderer, which turn their shapes into these calls.
	class sprite_pass
	{
	public:
		sprite_pass();

		sprite_pass(const sprite_pass&) = delete;
		sprite_pass& operator=(const sprite_pass&) = delete;

		//----------------------------------------------------------
		/// @brief Prepares for drawing a frame. Called by the engine.
		/// @param commands Command list of the frame being drawn.
		/// @param pipeline Resources the pass draws with.
		/// @param view_projection Maps positions to the screen.
		void begin(rhi::command_list& commands, const sprite_pipeline& pipeline, const mat4& view_projection);

		//----------------------------------------------------------
		/// @brief Draws one quad with the given texture, uv rect (offset xy +
		/// scale zw, in 0..1 texture space), transformation and tint.
		void submit_quad(const texture& quad_texture, vec4 uv_rect, vec2 position, vec2 size, float rotation_radians, color tint);

		//----------------------------------------------------------
		/// @brief Draws triangles built on the cpu (circles, triangles) in
		/// one flat color.
		void submit_triangles(std::span<const vec2> corners, color fill_color);

		//----------------------------------------------------------
		/// @brief Draws many sprites of one texture with a single draw call.
		void submit_instances(const texture& sprite_texture, std::span<const sprite_instance> instances);

		//----------------------------------------------------------
		/// @brief Returns the 1x1 white texture shapes are drawn with.
		const texture& get_white_texture() const;

		//----------------------------------------------------------
		/// @brief Changes how positions map to the screen for the next draws.
		void set_view_projection(const mat4& view_projection);

	private:
		//----------------------------------------------------------
		/// @brief What the command list is set up for; switching costs a
		/// few commands, so it only happens when the next draw needs it.
		enum class bound_state
		{
			nothing,
			sprite_quad,   // sprite pipeline + the unit quad
			sprite_shapes, // sprite pipeline + vertices built this frame
			instanced,     // instanced sprite pipeline + the unit quad
		};

		//----------------------------------------------------------
		/// @brief Sets the command list up for the given kind of draw,
		/// unless it still is.
		void bind(bound_state state);

		rhi::command_list* m_commands;         // not owned: the frame being drawn
		bound_state m_bound;                   // what m_commands is set up for
		const rhi::pipeline* m_bound_pipeline; // the pipeline bind set, to see whether another renderer replaced it

		// Reused by submit_instances so it does not allocate every call:
		// four vec4 per sprite, laid out as sprite_instanced_vs.hlsl reads them.
		std::vector<vec4> m_instance_data;
		const sprite_pipeline* m_pipeline; // not owned: what the pass draws with
		mat4 m_view_projection;            // maps positions to the screen
	};
}

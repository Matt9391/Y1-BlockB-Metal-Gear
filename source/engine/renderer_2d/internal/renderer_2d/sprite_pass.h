#pragma once

#include "core/math.h"
#include "renderer_2d/renderer_2d.h"
#include "renderer_2d/sprite_pipeline.h"

#include <span>
#include <vector>

namespace buz
{
	namespace rhi
	{
		class command_list;
		class pipeline;
	}

	struct render_context;
	class texture;

	//--------------------------------------------------------------
	/// @brief The engine's side of renderer_2d: the gpu work of one pass
	/// (the world or the screen). It records quads, triangles and instances
	/// into the frame's command list, and only switches the pipeline when
	/// the next draw needs another one. Games draw through world_renderer_2d
	/// and screen_renderer, which turn their shapes into these calls.
	///
	/// The game class that owns the renderer starts the pass with the
	/// frame's command list (begin_frame) before handing the renderer to the
	/// game, and ends it after.
	class sprite_pass
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates the pass and the sprite pipeline it draws with.
		/// @param context The engine's render context.
		/// @param sampler_filter How its textures are filtered when scaled
		/// on screen.
		sprite_pass(const render_context& context, texture_filter sampler_filter);
		~sprite_pass();

		sprite_pass(const sprite_pass&) = delete;
		sprite_pass& operator=(const sprite_pass&) = delete;

		//----------------------------------------------------------
		/// @brief Draws into this command list until end_frame, starting in
		/// window pixels (see set_view_projection).
		void begin_frame(rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Stops drawing into the frame begin_frame started.
		void end_frame();

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
		/// @brief Changes how positions map to the screen for the rest of
		/// the frame; every frame starts in window pixels.
		void set_view_projection(const mat4& view_projection);

	private:
		//----------------------------------------------------------
		/// @brief Asserts that the pass was started: the game draws only in
		/// the render function that gets the renderer.
		void assert_started() const;

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

		const render_context& m_context;       // the engine's: the device
		sprite_pipeline m_pipeline;            // what the pass draws with
		rhi::command_list* m_commands;         // not owned: the frame being drawn, nullptr outside it
		bound_state m_bound;                   // what m_commands is set up for
		const rhi::pipeline* m_bound_pipeline; // the pipeline bind set, to see whether another renderer replaced it

		// Reused by submit_instances so it does not allocate every call:
		// four vec4 per sprite, laid out as sprite_instanced_vs.hlsl reads them.
		std::vector<vec4> m_instance_data;
		mat4 m_view_projection; // maps positions to the screen
	};
}

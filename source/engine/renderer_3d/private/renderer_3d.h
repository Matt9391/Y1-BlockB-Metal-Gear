#pragma once

#include "core/primitives.h"
#include "render_core/render_context.h"
#include "renderer_3d/material.h"
#include "renderer_3d/surface_shading.h"
#include "renderer_3d/world_renderer_3d.h"

#include <array>
#include <cstddef>
#include <memory>
#include <span>
#include <vector>

namespace buz
{
	namespace rhi
	{
		class command_list;
		class pipeline;
	}

	class mesh;
	class texture;

	//--------------------------------------------------------------
	/// @brief How many built-in shapes there are (core/primitives.h).
	inline constexpr size_t k_primitive_count = static_cast<size_t>(primitive::octahedron) + 1;

	//--------------------------------------------------------------
	/// @brief One end of a debug line.
	struct line_vertex
	{
		vec3 position;    // in the world
		color line_color; // the line's color at this end
	};

	//--------------------------------------------------------------
	/// @brief The engine's side of world_renderer_3d: its pipelines, the
	/// built-in shapes, the camera and the debug views. It draws into the
	/// frame's command list before the screen renderer draws the hud on top.
	///
	/// Every mesh draw hands the shaders three blocks of constants: its own
	/// at register(b0) (world matrix, normal matrix, material), the game's
	/// shader data at register(b1), and the frame data at register(b2),
	/// which holds the camera. The camera is not in the per-draw constants
	/// because those are full at the 128 bytes every backend guarantees.
	class renderer_3d
	{
	public:
		explicit renderer_3d(const render_context& context);
		~renderer_3d();

		renderer_3d(const renderer_3d&) = delete;
		renderer_3d& operator=(const renderer_3d&) = delete;

		//----------------------------------------------------------
		/// @brief Draws into this command list until end_frame.
		void begin_frame(rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Stops drawing into the frame begin_frame started.
		void end_frame();

		//----------------------------------------------------------
		/// @brief Draws with these debug views from now on.
		void set_debug_views(const debug_views& views);

		//----------------------------------------------------------
		/// @brief Returns the debug views it draws with.
		const debug_views& get_debug_views() const;

		//----------------------------------------------------------
		/// @brief Sets the camera the next mesh draws are seen through
		/// (see world_renderer_3d::set_camera).
		void set_camera(const mat4& view, const mat4& projection);

		//----------------------------------------------------------
		/// @brief Draws a mesh with a material and a texture; a white
		/// texture when surface is nullptr.
		void draw(const mesh& shape, const material& look, const texture* surface, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws one of the built-in shapes, untextured.
		void draw(primitive shape, const material& look, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws debug lines, two vertices each, when debug lines are on.
		void draw_lines(std::span<const line_vertex> vertices);

		//----------------------------------------------------------
		/// @brief Draws meshes with other shaders from now on; does nothing
		/// when they are the ones in use.
		void set_surface_shading(const surface_shading& shading);

		//----------------------------------------------------------
		/// @brief Keeps a copy of the data the mesh shaders read at
		/// register(b1) (see world_renderer_3d::set_shader_data).
		void set_shader_data(std::span<const std::byte> data);

	private:
		//----------------------------------------------------------
		/// @brief Copies the shader data into the frame, the first time a
		/// frame draws a mesh and again after it changes.
		void upload_shader_data(rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Copies the camera into the frame (register(b2)), the
		/// first time a frame draws a mesh and again after it changes.
		void upload_frame_data(rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Returns the frame's command list; asserts that the game is
		/// drawing.
		rhi::command_list& get_commands() const;

		//----------------------------------------------------------
		/// @brief Returns the mesh pipeline for the current debug views.
		const rhi::pipeline& select_mesh_pipeline() const;

		const render_context& m_context; // the engine's: the device and the shaders
		rhi::command_list* m_commands;   // not owned: the frame being drawn, nullptr outside it
		debug_views m_debug_views;

		// Mesh pipelines: [wireframe], built for m_surface_shading
		std::unique_ptr<rhi::pipeline> m_mesh_pipelines[2];
		surface_shading m_surface_shading;
		std::unique_ptr<rhi::pipeline> m_line_pipeline;
		std::array<std::shared_ptr<mesh>, k_primitive_count> m_primitives;
		std::shared_ptr<texture> m_white_texture;

		mat4 m_view_projection; // the camera: world positions to the screen
		vec3 m_camera_position; // where the camera is in the world
		bool m_camera_uploaded; // copied into this frame, and not changed since

		std::vector<std::byte> m_shader_data; // set_shader_data's copy; empty before
		bool m_shader_data_uploaded;          // copied into this frame, and not changed since
	};
}

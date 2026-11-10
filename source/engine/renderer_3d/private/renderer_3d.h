#pragma once

#include "primitives.h"
#include "renderer_3d/material.h"
#include "renderer_3d/mesh_shading.h"
#include "renderer_3d/world_renderer_3d.h"
#include "rhi/render_context.h"

#include <array>
#include <memory>
#include <span>

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
	/// @brief Uploads triangles to the gpu; create_mesh for code that has
	/// the render context rather than the engine (the built-in shapes).
	std::shared_ptr<mesh> upload_mesh(const render_context& context, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices);

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
	/// engine's world pass, alongside the 2D world's sprites.
	class renderer_3d
	{
	public:
		explicit renderer_3d(const render_context& context);
		~renderer_3d();

		renderer_3d(const renderer_3d&) = delete;
		renderer_3d& operator=(const renderer_3d&) = delete;

		//----------------------------------------------------------
		/// @brief Sets how world positions map to the screen.
		void set_view_projection(const mat4& view_projection);

		//----------------------------------------------------------
		/// @brief Draws a mesh with a material.
		void draw(const mesh& shape, const material& look, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws one of the built-in shapes.
		void draw(primitive shape, const material& look, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws debug lines, two vertices each, when debug lines are on.
		void draw_lines(std::span<const line_vertex> vertices);

		//----------------------------------------------------------
		/// @brief Switches between solid triangles and their edges only.
		void toggle_wireframe();

		//----------------------------------------------------------
		/// @brief Switches the lighting off and on again.
		void toggle_lighting();

		//----------------------------------------------------------
		/// @brief Switches the debug lines off and on again.
		void toggle_debug_lines();

		//----------------------------------------------------------
		/// @brief Draws meshes with other shaders from now on.
		void set_mesh_shading(const mesh_shading& shading);

	private:
		//----------------------------------------------------------
		/// @brief Returns the world pass's command list; asserts that the
		/// game is drawing its world.
		rhi::command_list& get_commands() const;

		//----------------------------------------------------------
		/// @brief Returns the mesh pipeline for the current debug views.
		const rhi::pipeline& select_mesh_pipeline() const;

		const render_context& m_context; // the engine's: the device, and the world pass while it runs

		// Mesh pipelines: [wireframe]
		std::unique_ptr<rhi::pipeline> m_mesh_pipelines[2];
		std::unique_ptr<rhi::pipeline> m_line_pipeline;
		std::array<std::shared_ptr<mesh>, static_cast<size_t>(primitive::count)> m_primitives;
		std::shared_ptr<texture> m_white_texture;

		mat4 m_view_projection; // the camera: world positions to the screen

		// Debug views
		bool m_wireframe;    // triangle edges only
		bool m_lighting_off; // every surface at its plain color
		bool m_show_debug;   // debug lines drawn
	};
}

/// @file
/// @brief Draws the 3D game world: meshes, the built-in shapes and debug lines.

#pragma once

#include "core/math.h"
#include "renderer_3d/material.h"

#include <memory>

namespace buz
{
	class engine;
	class mesh;
	class renderer_3d;

	//--------------------------------------------------------------
	/// @brief Draws the 3D game world, through a camera.
	///
	/// Your game makes one in its constructor and draws with it in
	/// on_render_world; the world is drawn in the order of your calls, and
	/// nearer surfaces hide farther ones whatever that order (there is a
	/// depth buffer).
	/// @code{.cpp}
	/// my_game::my_game(buz::engine& engine)
	///     : m_world_3d(engine)
	/// {
	/// }
	///
	/// void my_game::on_render_world(buz::engine& engine, buz::world_renderer_2d& world)
	/// {
	///     m_world_3d.draw_box({ 0.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, m_crate_material);
	///     m_world_3d.draw(*m_snake_mesh, m_snake_material, m_snake_transform);
	/// }
	/// @endcode
	///
	/// Until your game sets a camera, the world is seen from a fixed spot up,
	/// behind and to the side of the origin, looking at it.
	class world_renderer_3d
	{
		//----------------------------------------------------------
		// Lifetime
	public:
		//----------------------------------------------------------
		/// @brief Creates the pipelines and the built-in shapes on the
		/// engine's graphics card.
		explicit world_renderer_3d(engine& engine);
		~world_renderer_3d();

		world_renderer_3d(const world_renderer_3d&) = delete;
		world_renderer_3d& operator=(const world_renderer_3d&) = delete;

		//----------------------------------------------------------
		// Camera
	public:
		//----------------------------------------------------------
		/// @brief Sets the camera for the draws that follow, until it is set
		/// again. Building the two matrices is yours to do (see Drawing in
		/// 3D for the conventions they follow).
		/// @param view Moves the world so the camera sits at the origin,
		/// looking down -z with y up.
		/// @param projection Maps what the camera sees onto the window, with
		/// depth from 0 (near) to 1 (far).
		void set_camera(const mat4& view, const mat4& projection);

		//----------------------------------------------------------
		// Drawing; only while game::on_render_world runs
	public:
		//----------------------------------------------------------
		/// @brief Draws a mesh.
		/// @param shape The mesh.
		/// @param look Its material.
		/// @param transform Where it is: object space to world space.
		void draw(const mesh& shape, const material& look, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws a box. Handy for greyboxing a level.
		/// @param position Its center.
		/// @param size Its width (x), height (y) and depth (z).
		/// @param look Its material.
		void draw_box(vec3 position, vec3 size, const material& look);

		//----------------------------------------------------------
		/// @brief Draws a sphere, or an ellipsoid when size differs per axis.
		/// @param position Its center.
		/// @param size Its diameter along x, y and z.
		/// @param look Its material.
		void draw_sphere(vec3 position, vec3 size, const material& look);

		//----------------------------------------------------------
		/// @brief Draws a flat rectangle in the xz plane, facing up (+y): a
		/// floor. Only its top side is drawn.
		/// @param position Its center.
		/// @param size Its width (x) and depth (z); y is ignored.
		/// @param look Its material.
		void draw_plane(vec3 position, vec3 size, const material& look);

		//----------------------------------------------------------
		/// @brief Draws a cylinder standing along y.
		/// @param position Its center.
		/// @param size Its diameter along x and z, and its height (y).
		/// @param look Its material.
		void draw_cylinder(vec3 position, vec3 size, const material& look);

		//----------------------------------------------------------
		/// @brief Draws an octahedron (two pyramids back to back): pickups,
		/// markers.
		/// @param position Its center.
		/// @param size Its width, height and depth, corner to corner.
		/// @param look Its material.
		void draw_octahedron(vec3 position, vec3 size, const material& look);

		//----------------------------------------------------------
		// Debug views
	public:
		//----------------------------------------------------------
		/// @brief Switches between solid triangles and their edges only.
		void toggle_wireframe();

		//----------------------------------------------------------
		/// @brief Switches the lighting off (every surface at its plain
		/// color) and on again.
		void toggle_lighting();

		//----------------------------------------------------------
		/// @brief Switches the debug lines (see debug_draw.h) off and on
		/// again; they start on.
		void toggle_debug_lines();

		//----------------------------------------------------------
		// For engine modules
	public:
		//----------------------------------------------------------
		/// @brief Returns the engine's side of this renderer (debug_draw
		/// draws its lines with it, set_mesh_shading changes its shaders). A game cannot use it: the type lives
		/// in the module's private headers.
		renderer_3d& get_renderer() const;

	private:
		std::unique_ptr<renderer_3d> m_renderer; // the pipelines, the built-in shapes and the camera
	};
}

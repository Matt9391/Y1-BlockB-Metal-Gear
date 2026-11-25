/// @file
/// @brief Draws the 3D game world: meshes, the built-in shapes and debug lines.

#pragma once

#include "core/math.h"
#include "renderer_3d/material.h"

#include <yes_no.h>

#include <cstddef>
#include <memory>
#include <span>

namespace buz
{
	namespace rhi
	{
		class command_list;
	}

	struct render_context;
	class mesh;
	class renderer_3d;
	class texture;

	//--------------------------------------------------------------
	/// @brief Whether meshes are drawn as the edges of their triangles only.
	using wireframe_view = utl::yes_no_t<struct wireframe_view_tag>;

	//--------------------------------------------------------------
	/// @brief Whether surfaces are lit, or all drawn at their plain color.
	using lighting_view = utl::yes_no_t<struct lighting_view_tag>;

	//--------------------------------------------------------------
	/// @brief Whether debug lines are drawn.
	using debug_lines_view = utl::yes_no_t<struct debug_lines_view_tag>;

	//--------------------------------------------------------------
	/// @brief The debug views a world_renderer_3d draws with. buz::game_3d
	/// switches them with the engine's debug keys
	/// (engine_settings::debug_controls), so a game does not handle them.
	struct debug_views
	{
		wireframe_view wireframe = wireframe_view::no;        // triangle edges only
		lighting_view lighting = lighting_view::yes;          // off: every surface at its plain color
		debug_lines_view debug_lines = debug_lines_view::yes; // the lines of debug_draw
	};

	//--------------------------------------------------------------
	/// @brief Draws the 3D game world, through a camera.
	///
	/// Owned by buz::game_3d and handed to on_render_world every frame; the
	/// world is drawn in the order of your calls, and nearer surfaces hide
	/// farther ones whatever that order (there is a depth buffer).
	/// @code{.cpp}
	/// void my_game::on_render_world(buz::engine& engine, buz::world_renderer_3d& world)
	/// {
	///     world.draw_box({ 0.0f, 0.5f, 0.0f }, { 1.0f, 1.0f, 1.0f }, m_crate_material);
	///     world.draw(*m_snake_mesh, m_snake_material, *m_snake_texture, m_snake_transform);
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
		/// @param context The engine's render context
		/// (engine::get_render_context).
		explicit world_renderer_3d(const render_context& context);
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
		// Drawing; only while on_render_world runs
	public:
		//----------------------------------------------------------
		/// @brief Draws a mesh in its material's plain color, untextured.
		/// @param shape The mesh.
		/// @param look Its material; its base_texture is not looked at.
		/// @param transform Where it is: object space to world space.
		void draw(const mesh& shape, const material& look, const mat4& transform);

		//----------------------------------------------------------
		/// @brief Draws a mesh with a texture, times the material's color.
		/// The material only names its image (material::base_texture);
		/// loading that image is yours (buz::load_texture), and this draws
		/// with the texture you hand it.
		/// @param shape The mesh.
		/// @param look Its material.
		/// @param surface The texture.
		/// @param transform Where it is: object space to world space.
		void draw(const mesh& shape, const material& look, const texture& surface, const mat4& transform);

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
		// Shader data
	public:
		//----------------------------------------------------------
		/// @brief Hands data to your own mesh shaders (see
		/// buz::set_surface_shading), for every draw that follows, in this
		/// frame and the next ones, until you set it again. The shaders read
		/// it as a cbuffer at register(b1):
		/// @code{.hlsl}
		/// [[vk::binding(0, 2)]] cbuffer shader_data : register(b1)
		/// {
		///     float4 fog_color;
		///     float fog_distance;
		/// };
		/// @endcode
		/// The struct you pass and the cbuffer must agree on the layout: hlsl
		/// packs the members in rows of four floats, and a member never
		/// straddles two rows. The built-in shaders don't read it.
		///
		/// The engine fills the other two blocks: the draw's `world` matrix,
		/// normal matrix, `lit`, `alpha_cutoff` and `base_color` at
		/// register(b0), and the camera's `view_projection` and
		/// `camera_position` at register(b2), with [[vk::binding(0, 3)]]
		/// (buz::surface_shading lists them; the diffuse shaders declare
		/// them).
		/// @param data At most 4096 bytes; a copy is kept.
		void set_shader_data(std::span<const std::byte> data);

		//----------------------------------------------------------
		// For engine modules
	public:
		//----------------------------------------------------------
		/// @brief Draws into this command list until end_frame; buz::game_3d
		/// calls it before on_render_world. A game cannot call it:
		/// rhi::command_list is defined in the engine's internal headers.
		/// @param commands The frame's command list.
		void begin_frame(rhi::command_list& commands);

		//----------------------------------------------------------
		/// @brief Stops drawing into the frame begin_frame started;
		/// buz::game_3d calls it after on_render_world.
		void end_frame();

		//----------------------------------------------------------
		/// @brief Draws with these debug views from now on; buz::game_3d
		/// switches them with the engine's debug keys.
		void set_debug_views(const debug_views& views);

		//----------------------------------------------------------
		/// @brief Returns the debug views it draws with.
		const debug_views& get_debug_views() const;

		//----------------------------------------------------------
		/// @brief Returns the engine's side of this renderer (debug_draw
		/// draws its lines with it, set_surface_shading changes its
		/// shaders). A game cannot use it: renderer_3d is defined in the
		/// module's private headers.
		renderer_3d& get_renderer() const;

	private:
		std::unique_ptr<renderer_3d> m_renderer; // the pipelines, the built-in shapes and the camera
	};
}

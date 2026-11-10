/// @file
/// @brief Draws the 2D game world, through the camera.

#pragma once

#include "renderer_2d/renderer_2d.h"

namespace buz
{
	class sprite_pass;

	//--------------------------------------------------------------
	/// @brief Draws the 2D game world: everything that lives in the level
	/// and moves when the camera moves.
	///
	/// Handed to game::on_render_world every frame. Until you call set_camera,
	/// world positions are plain window pixels, with (0, 0) in the top-left
	/// corner. A camera is two matrices: the view (where the camera is) and
	/// the projection (how the world maps onto the window). Scrolling a
	/// level to follow the player is a view that moves:
	/// @code{.cpp}
	/// const buz::mat4 view = glm::translate(buz::mat4(1.0f), buz::vec3(-scroll_x, 0.0f, 0.0f));
	/// const buz::mat4 projection = buz::make_orthographic_projection(1280.0f, 720.0f);
	/// world.set_camera(view, projection);
	/// @endcode
	///
	/// A 3D game draws its world with a buz::world_renderer_3d instead
	/// (renderer_3d/world_renderer_3d.h).
	class world_renderer_2d : public renderer_2d
	{
		//----------------------------------------------------------
		// Lifetime
	public:
		//----------------------------------------------------------
		/// @brief Draws into the engine's world pass; only the engine has
		/// one to pass in.
		explicit world_renderer_2d(sprite_pass& pass);

		//----------------------------------------------------------
		// Camera
	public:
		//----------------------------------------------------------
		/// @brief Sets the camera for the rest of this frame.
		/// @param view Moves the world so the camera sits at the origin.
		/// @param projection Maps what the camera sees onto the window.
		void set_camera(const mat4& view, const mat4& projection);

	private:
		sprite_pass& m_pass; // the engine's side: the pass and its camera
	};
}

/// @file
/// @brief A 2D image in gpu memory, ready to be drawn.

#pragma once

#include "core/math.h"

#include <cstdint>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief A 2d image living in gpu memory, ready to be drawn.
	///
	/// The engine makes textures: buz::load_texture reads an image file,
	/// buz::create_texture takes pixels made in code (engine/texture_loading.h).
	/// Both hand them out as a std::shared_ptr, so you never new/delete a
	/// texture yourself.
	class texture
	{
	public:
		texture(const texture&) = delete;
		texture& operator=(const texture&) = delete;

		//----------------------------------------------------------
		/// @brief Returns the width of the texture in pixels.
		uint32_t get_width() const;

		//----------------------------------------------------------
		/// @brief Returns the height of the texture in pixels.
		uint32_t get_height() const;

		//----------------------------------------------------------
		/// @brief Returns the width and height of the texture in pixels.
		vec2 get_size() const;

	protected:
		// Only the engine makes textures (rhi::gpu_texture, its side of this
		// class, holds the gpu memory)
		texture(uint32_t width, uint32_t height);
		~texture();

	private:
		uint32_t m_width;  // in pixels
		uint32_t m_height; // in pixels
	};
}

/// @file
/// @brief A 2D image in gpu memory, ready to be drawn.

#pragma once

#include "core/math.h"

#include <cstdint>
#include <memory>

namespace buz
{
	namespace rhi
	{
		class gpu_texture;
	}

	//--------------------------------------------------------------
	/// @brief A 2d image living in gpu memory, ready to be drawn.
	///
	/// The engine makes textures: buz::load_texture reads an image file,
	/// buz::create_texture takes pixels made in code (asset_management/texture_loading.h).
	/// Both hand them out as a std::shared_ptr, so you never new/delete a
	/// texture yourself.
	class texture
	{
	public:
		//----------------------------------------------------------
		/// @brief Wraps an image the engine uploaded. Games get their
		/// textures from load_texture and create_texture instead.
		explicit texture(std::unique_ptr<rhi::gpu_texture> image);
		~texture();

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

		//----------------------------------------------------------
		/// @brief Returns the image in gpu memory, for the renderers. Only
		/// the engine can use it: everywhere else rhi::gpu_texture is an
		/// incomplete type.
		const rhi::gpu_texture& get_gpu_texture() const;

	private:
		std::unique_ptr<rhi::gpu_texture> m_image; // the image in gpu memory, and its size
	};
}

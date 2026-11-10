#pragma once

#include "rhi/texture.h"

#include <cstdint>
#include <memory>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief The engine's side of a texture: an rgba image in gpu memory
	/// that shaders can sample. Every buz::texture a game holds is one.
	class gpu_texture final : public texture
	{
	public:
		class native; // the backend's data: the image and its descriptor

		gpu_texture(uint32_t width, uint32_t height, std::unique_ptr<native> data);
		~gpu_texture();

		//----------------------------------------------------------
		/// @brief Returns the backend's data. Only the backend can use
		/// it: everywhere else native is an incomplete type.
		const native& get_native() const;

	private:
		std::unique_ptr<native> m_native; // the backend's image and descriptor
	};

	//--------------------------------------------------------------
	/// @brief Returns the gpu side of a texture a game handed to the engine.
	const gpu_texture& to_gpu_texture(const texture& image);
}

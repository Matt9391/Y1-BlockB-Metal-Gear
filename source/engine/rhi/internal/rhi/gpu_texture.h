#pragma once

#include <cstdint>
#include <memory>

namespace buz::rhi
{
	//--------------------------------------------------------------
	/// @brief An rgba image in gpu memory that shaders can sample. Every
	/// buz::texture a game holds owns one.
	class gpu_texture final
	{
	public:
		class native; // the backend's data: the image and its descriptor

		gpu_texture(uint32_t width, uint32_t height, std::unique_ptr<native> data);
		~gpu_texture();

		gpu_texture(const gpu_texture&) = delete;
		gpu_texture& operator=(const gpu_texture&) = delete;

		//----------------------------------------------------------
		/// @brief Returns the width of the image in pixels.
		uint32_t get_width() const;

		//----------------------------------------------------------
		/// @brief Returns the height of the image in pixels.
		uint32_t get_height() const;

		//----------------------------------------------------------
		/// @brief Returns the backend's data. Only the backend can use
		/// it: everywhere else native is an incomplete type.
		const native& get_native() const;

	private:
		uint32_t m_width;                // in pixels
		uint32_t m_height;               // in pixels
		std::unique_ptr<native> m_native; // the backend's image and descriptor
	};
}

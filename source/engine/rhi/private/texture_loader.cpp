#include "rhi/texture_loader.h"

#include "core/log.h"
#include "rhi/device.h"
#include "rhi/gpu_texture.h"

#include <stb/stb_image.h>

#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	texture_loader::texture_loader()
		: m_device(nullptr)
	{
	}

	//--------------------------------------------------------------
	void texture_loader::initialize(rhi::device& graphics_device)
	{
		m_device = &graphics_device;
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> texture_loader::load_from_file(const fs::path& path)
	{
		// Decode the image file into raw rgba pixels (8 bits per channel)
		int width = 0;
		int height = 0;
		int channels_in_file = 0;

		stbi_uc* pixels = stbi_load(path.string().c_str(), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			// A wrong path is the usual cause here. Rather than hand back a
			// nullptr that blows up the next draw_sprite call, log a clear
			// error and return the visible placeholder so the game keeps
			// running and the mistake shows up on screen.
			log::error("Failed to load image \"{}\": {}. Using placeholder texture.", path.string(), stbi_failure_reason());
			return get_placeholder_texture();
		}

		const uint32_t texture_width = static_cast<uint32_t>(width);
		const uint32_t texture_height = static_cast<uint32_t>(height);
		const std::span<const uint8_t> rgba_pixels(pixels, static_cast<size_t>(width) * height * 4);
		std::shared_ptr<texture> loaded_texture = create_from_pixels(texture_width, texture_height, rgba_pixels);
		stbi_image_free(pixels);

		if (loaded_texture == nullptr)
		{
			// The file decoded fine but the gpu upload failed; the placeholder
			// keeps callers safe from a nullptr just the same.
			log::error("Failed to upload texture \"{}\" to the gpu. Using placeholder texture.", path.string());
			return get_placeholder_texture();
		}

		log::info("Loaded texture \"{}\" ({}x{})", path.string(), width, height);

		return loaded_texture;
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> texture_loader::load_from_memory(std::span<const uint8_t> encoded_image, const std::string& name)
	{
		int width = 0;
		int height = 0;
		int channels_in_file = 0;

		stbi_uc* pixels = stbi_load_from_memory(encoded_image.data(), static_cast<int>(encoded_image.size()), &width, &height, &channels_in_file, STBI_rgb_alpha);
		if (pixels == nullptr)
		{
			log::error("Failed to decode image \"{}\": {}. Using placeholder texture.", name, stbi_failure_reason());
			return get_placeholder_texture();
		}

		const uint32_t texture_width = static_cast<uint32_t>(width);
		const uint32_t texture_height = static_cast<uint32_t>(height);
		const std::span<const uint8_t> rgba_pixels(pixels, static_cast<size_t>(width) * height * 4);
		std::shared_ptr<texture> loaded_texture = create_from_pixels(texture_width, texture_height, rgba_pixels);
		stbi_image_free(pixels);

		return loaded_texture != nullptr ? loaded_texture : get_placeholder_texture();
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> texture_loader::get_placeholder_texture()
	{
		if (m_placeholder_texture != nullptr)
		{
			return m_placeholder_texture;
		}

		// A magenta-and-black checkerboard: unmistakably "not a real texture",
		// so a missing image is obvious at a glance instead of blending in.
		constexpr uint32_t size = 16;
		constexpr uint32_t cell = size / 2;

		std::vector<uint8_t> pixels(static_cast<size_t>(size) * size * 4);
		for (uint32_t y = 0; y < size; ++y)
		{
			for (uint32_t x = 0; x < size; ++x)
			{
				const bool magenta = ((x / cell) + (y / cell)) % 2 == 0;

				uint8_t* pixel = &pixels[(static_cast<size_t>(y) * size + x) * 4];
				pixel[0] = magenta ? 255 : 0; // r
				pixel[1] = 0;                 // g
				pixel[2] = magenta ? 255 : 0; // b
				pixel[3] = 255;               // a
			}
		}

		m_placeholder_texture = create_from_pixels(size, size, pixels);
		return m_placeholder_texture;
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> texture_loader::create_from_pixels(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels)
	{
		// The device checks the pixel data is width * height * 4
		// bytes: the span carries its size, which a raw pointer could not.
		// The shared_ptr takes the texture over from the unique_ptr (or
		// stays empty when uploading failed).
		return m_device->create_texture(width, height, rgba_pixels);
	}
}

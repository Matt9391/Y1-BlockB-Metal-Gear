#pragma once

#include "core/filesystem.h"
#include "rhi/texture.h"

#include <memory>
#include <span>
#include <string>
#include <vector>

namespace buz
{
	namespace rhi
	{
		class device;
	}

	//--------------------------------------------------------------
	/// @brief Imports image files and uploads them to the gpu as textures.
	///
	/// Uses stb_image to decode the file (png, jpg, bmp, tga, ...) into raw
	/// rgba pixels, then hands those to the graphics device, which copies
	/// them into gpu memory.
	class texture_loader
	{
	public:
		//----------------------------------------------------------
		/// @brief Creates a loader; initialize gives it the device.
		texture_loader();

		//----------------------------------------------------------
		/// @brief Prepares the loader for uploading textures.
		/// @param graphics_device Device the textures are created on.
		void initialize(rhi::device& graphics_device);

		//----------------------------------------------------------
		/// @brief Loads an image file and uploads it to the gpu.
		///
		/// When the file cannot be loaded (a wrong path is the usual cause) an
		/// error is logged and a magenta-and-black checkerboard placeholder is
		/// returned instead of nullptr, so the game keeps running and the
		/// missing texture shows up on screen instead of crashing a draw call.
		/// @param path Path to the image file.
		/// @return The uploaded texture, or the placeholder when loading failed.
		std::shared_ptr<texture> load_from_file(const fs::path& path);

		//----------------------------------------------------------
		/// @brief Decodes an image file that is already in memory (a png
		/// embedded in a .glb model, say) and uploads it to the gpu.
		/// @param encoded_image The bytes of the image file.
		/// @param name Used in error messages.
		/// @return The uploaded texture, or the placeholder when decoding failed.
		std::shared_ptr<texture> load_from_memory(std::span<const uint8_t> encoded_image, const std::string& name);

		//----------------------------------------------------------
		/// @brief Uploads raw pixels to the gpu, e.g. for procedurally
		/// generated textures.
		///
		/// A std::span is a view over contiguous memory that knows its own
		/// size; a std::vector or a plain array converts to it implicitly.
		/// Because the span carries its length, the loader can verify the
		/// pixel data matches width * height, which a raw pointer
		/// could never check.
		/// @param width Width of the texture in pixels.
		/// @param height Height of the texture in pixels.
		/// @param rgba_pixels width * height pixels, 4 bytes (r, g, b, a) each.
		/// @return The uploaded texture, or nullptr when uploading failed.
		std::shared_ptr<texture> create_from_pixels(uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels);

	private:
		//----------------------------------------------------------
		/// @brief Returns the shared magenta-and-black placeholder texture,
		/// creating it on first use.
		///
		/// The placeholder is built once and reused for every failed load: the
		/// DirectX 12 backend never reuses descriptor slots, so making a fresh texture per
		/// failure would slowly exhaust it.
		/// @return The placeholder texture, or nullptr when even that could not be created.
		std::shared_ptr<texture> get_placeholder_texture();

		rhi::device* m_device; // not owned; set by initialize

		// Magenta-and-black checkerboard returned when a load fails; created lazily
		std::shared_ptr<texture> m_placeholder_texture;
	};
}

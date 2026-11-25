#pragma once

#include <cstdint>
#include <span>
#include <string>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Where the image a texture_id names comes from. asset_management
	/// makes these while it imports a model, and reads them back to load
	/// the image.
	struct texture_source
	{
		uint64_t hash = 0; // of the image's bytes; never 0
		std::string what;  // for messages
		std::string path;  // the image file, relative to the content folder; empty for an image inside a model

		// The encoded image (a png, say) of an image inside a model, kept
		// so loading it needs no second look at the model file; empty for
		// an image file
		std::vector<uint8_t> embedded_image;
	};

	//--------------------------------------------------------------
	/// @brief The hash texture ids compare images by: 64 bit FNV-1a over
	/// the encoded bytes. Never 0, which stands for no image.
	uint64_t hash_image_bytes(std::span<const uint8_t> bytes);
}

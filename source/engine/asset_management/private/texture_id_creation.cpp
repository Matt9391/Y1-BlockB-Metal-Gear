#include "texture_id_creation.h"

#include "core/texture_source.h"

#include <memory>

namespace buz
{
	//--------------------------------------------------------------
	texture_id make_embedded_texture_id(const std::string& description, std::vector<uint8_t> encoded_image)
	{
		auto source = std::make_shared<texture_source>();
		source->hash = hash_image_bytes(encoded_image);
		source->what = description;
		source->embedded_image = std::move(encoded_image);
		return texture_id(std::move(source));
	}
}

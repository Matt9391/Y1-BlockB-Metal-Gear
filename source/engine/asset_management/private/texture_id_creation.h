#pragma once

#include "core/texture_id.h"

#include <cstdint>
#include <string>
#include <vector>

namespace buz
{
	//--------------------------------------------------------------
	/// @brief Names an image inside a model file, and keeps its bytes.
	/// (An image file is named with texture_id's own constructor.)
	/// @param description What texture_id::what returns.
	texture_id make_embedded_texture_id(const std::string& description, std::vector<uint8_t> encoded_image);
}

#include "engine/texture_loading.h"

#include "core/paths.h"
#include "engine/engine.h"
#include "rhi/render_context.h"
#include "rhi/texture_loader.h"

namespace buz
{
	//--------------------------------------------------------------
	std::shared_ptr<texture> load_texture(engine& engine, std::string_view content_relative_path)
	{
		return engine.get_render_context().textures->load_from_file(paths::get_content_path(content_relative_path));
	}

	//--------------------------------------------------------------
	std::shared_ptr<texture> create_texture(engine& engine, uint32_t width, uint32_t height, std::span<const uint8_t> rgba_pixels)
	{
		return engine.get_render_context().textures->create_from_pixels(width, height, rgba_pixels);
	}
}

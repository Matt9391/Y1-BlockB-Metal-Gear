#include "asset_management/model_loading.h"

#include "engine/engine.h"
#include "renderer_3d/model_loading.h"

namespace buz
{
	//--------------------------------------------------------------
	std::optional<model> load_model(engine& engine, std::string_view content_relative_path)
	{
		return load_model(engine.get_render_context(), content_relative_path);
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> load_mesh(engine& engine, std::string_view content_relative_path)
	{
		return load_mesh(engine.get_render_context(), content_relative_path);
	}

	//--------------------------------------------------------------
	std::shared_ptr<mesh> create_mesh(engine& engine, std::span<const mesh_vertex> vertices, std::span<const uint32_t> indices)
	{
		return create_mesh(engine.get_render_context(), vertices, indices);
	}
}

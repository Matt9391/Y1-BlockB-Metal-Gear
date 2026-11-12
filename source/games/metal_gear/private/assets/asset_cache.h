#pragma once

#include "engine/engine.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"
#include "renderer_2d/font.h"
#include "engine/texture_loading.h"

#include <string>
#include <unordered_map>

class asset_cache
{

public:
	asset_cache(buz::engine& engine);
	~asset_cache();

	const buz::model& get_model(const std::string& key);
	const buz::font& get_font(const std::string& key);
	std::shared_ptr<buz::texture> get_texture(const std::string& key);


private:
	buz::engine& m_engine;

	std::unordered_map<std::string, buz::model> m_cache_model;
	std::unordered_map<std::string, std::shared_ptr<buz::texture>> m_cache_texture;
	std::unordered_map<std::string, buz::font> m_cache_font;
};

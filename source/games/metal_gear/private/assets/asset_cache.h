#pragma once

#include "engine/engine.h"
#include "renderer_3d/mesh.h"
#include "renderer_3d/model.h"
#include "renderer_2d/font.h"
#include "asset_management/texture_loading.h"

#include "textured_model.h"

#include <string>
#include <unordered_map>	


class asset_cache
{

public:
	asset_cache(buz::engine& engine);
	~asset_cache() = default;
    
    asset_cache(const asset_cache& other) = delete;


	const textured_model& get_textured_model(const std::string& key);
	const buz::font& get_font(const std::string& key);
	const std::shared_ptr<buz::texture>& get_texture(const buz::texture_id& key);

private:
	
	std::vector<std::shared_ptr<buz::texture>> get_model_textures(const buz::model& model);
	
	
	buz::engine& m_engine;

	std::unordered_map<std::string, textured_model> m_cache_texture_model;
	std::unordered_map<buz::texture_id, std::shared_ptr<buz::texture>> m_cache_texture;
	std::unordered_map<std::string, buz::font> m_cache_font;

};

#include "asset_cache.h"

#include "asset_management/model_loading.h"
#include "core/log.h"
#include "core/assert.h"

asset_cache::asset_cache(buz::engine& engine)
	: m_engine{ engine }
{
}


const textured_model& asset_cache::get_textured_model(const std::string& key)
{

	auto it = m_cache_texture_model.find(key);

	if (it != m_cache_texture_model.end())
	{
		buz::log::info("return cached model from {}", key);
		return it->second;
	}
	
    std::optional<buz::model> model = buz::load_model(m_engine, key);
    if (model.has_value())
	{
		std::vector<std::shared_ptr<buz::texture>> textures = get_model_textures(*model);

		textured_model t_textured_model = {std::move(*model), std::move(textures)};
		m_cache_texture_model.emplace(key, std::move(t_textured_model));
		buz::log::info("Loaded new model from {}", key);
		return m_cache_texture_model.at(key);
	}

    buz::log::error("Could not load the model from {}", key);
    BUZ_ASSERT_MSG(model.has_value(), "Could not load the model from {}", key);
    buz::fatal_error("Could not load the model from {}", key);

	//* never reach here hopefully
}

const buz::font& asset_cache::get_font(const std::string& key)
{

	auto it = m_cache_font.find(key);

	if (it != m_cache_font.end())
	{
		buz::log::info("return cached font from {}", key);
		return it->second;
	}

    m_cache_font[key].atlas = get_texture(buz::texture_id(key));
    buz::log::info("return new font from {}", key);

    return m_cache_font.at(key);

}

//DONE: shouldnt I return a reference here? 
const std::shared_ptr<buz::texture>& asset_cache::get_texture(const buz::texture_id& key)
{

    auto it = m_cache_texture.find(key);

	if (it != m_cache_texture.end())
	{
		buz::log::info("return cached texture from {}", key.what());
		return it->second;
	}

    m_cache_texture[key] = buz::load_texture(m_engine, key);
    buz::log::info("return new texture from {}", key.what());

    return m_cache_texture.at(key);

}


std::vector<std::shared_ptr<buz::texture>> asset_cache::get_model_textures(const buz::model& model){
	std::vector<std::shared_ptr<buz::texture>> textures;
	textures.reserve(model.nodes.size());

	for (const auto& node : model.nodes){

		textures.push_back(node.look.base_texture.has_image()
        ? get_texture(node.look.base_texture)
        : nullptr);
	}

	return textures;
}

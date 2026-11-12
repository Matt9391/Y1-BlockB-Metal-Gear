#include "asset_cache.h"

#include "core/log.h"
#include "core/assert.h"

asset_cache::asset_cache(buz::engine& engine)
	: m_engine{ engine }
{
}

asset_cache::~asset_cache()
{
}

const buz::model& asset_cache::get_model(const std::string& key)
{
	static const buz::model k_missing_model; // static so that its always the same

	auto it = m_cache_model.find(key);

	if (it != m_cache_model.end())
	{
		buz::log::info("return cached model from {}", key);
		return it->second;
	}
	
    std::optional<buz::model> model = buz::load_model(m_engine, key);
	
    if (model.has_value())
	{
		m_cache_model.emplace(key, std::move(*model));
		buz::log::info("Loaded new model from {}", key);
		return m_cache_model.at(key);
	}

    buz::log::error("Could not load the model from {}", key);
    BUZ_ASSERT_MSG(model.has_value(), "Could not load the model from {}", key);
    return k_missing_model;

}

const buz::font& asset_cache::get_font(const std::string& key)
{

	auto it = m_cache_font.find(key);

	if (it != m_cache_font.end())
	{
		buz::log::info("return cached font from {}", key);
		return it->second;
	}

    m_cache_font[key].atlas = get_texture(key);
    buz::log::info("return new font from {}", key);

    return m_cache_font.at(key);

}

std::shared_ptr<buz::texture> asset_cache::get_texture(const std::string& key)
{

    auto it = m_cache_texture.find(key);

	if (it != m_cache_texture.end())
	{
		buz::log::info("return cached texture from {}", key);
		return it->second;
	}

    m_cache_texture[key] = buz::load_texture(m_engine, key);
    buz::log::info("return new texture from {}", key);
    
    return m_cache_texture.at(key);

}

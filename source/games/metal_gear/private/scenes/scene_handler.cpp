#include "scene_handler.h"
#include "scene_menu.h"
#include "scene_mission.h"
#include "scene_game_complete.h"
#include "scene_game_over.h"

scene_handler::scene_handler(scene_type startup_scene, asset_cache& asset_cache_, game_settings& game_settings_)
	: m_asset_cache(asset_cache_)
	, m_game_settings(game_settings_)
{
	set_scene(startup_scene);
}

void scene_handler::switch_scene()
{
	set_scene(m_current_scene->get_next_scene_type());
}

bool scene_handler::has_pending_scene()
{
	return m_current_scene->get_invoke_next_scene();
}

scene& scene_handler::get_current_scene()
{
	return *m_current_scene;
}

void scene_handler::set_scene(scene_type scene_t)
{
	switch (scene_t)
	{
	case scene_type::menu:
		m_current_scene = std::make_unique<scene_menu>(scene_type::menu,
													  scene_type::mission,
													  m_asset_cache
													  );
		break;
	case scene_type::mission:
		m_current_scene = std::make_unique<scene_mission>(scene_type::mission,
													  scene_type::game_complete,
													  m_asset_cache,
													  m_game_settings);
		break;
	case scene_type::game_complete:
		m_current_scene = std::make_unique<scene_game_complete>(scene_type::game_complete,
													  scene_type::menu,
													  m_asset_cache
													  );
		break;
	case scene_type::game_over:
		m_current_scene = std::make_unique<scene_game_over>(scene_type::game_over,
													  scene_type::menu,
													  m_asset_cache
													  );
		break;
	default:
		m_current_scene = std::make_unique<scene>(scene_type::game_complete,
													  scene_type::menu,
													  m_asset_cache
													  );
		break;
	}
}

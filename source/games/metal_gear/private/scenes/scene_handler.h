#pragma once
#include <memory>

#include "scene.h"
#include "assets/asset_cache.h"
#include "settings/game_settings.h"

class scene_handler {
    public:
    scene_handler(scene_type startup_scene, asset_cache& asset_cache_, game_settings& game_settings_);
    scene_handler() = delete;
    scene_handler(const scene_handler& other) = delete;

    ~scene_handler() = default;

    void switch_scene();

    bool has_pending_scene();

    scene& get_current_scene();

    private:

    void set_scene(scene_type scene_t);

    std::unique_ptr<scene> m_current_scene;

    asset_cache& m_asset_cache;
    game_settings& m_game_settings;
};


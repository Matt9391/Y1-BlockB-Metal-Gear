#pragma once
#include <memory>

#include "scene.h"
#include "assets/asset_cache.h"

class scene_handler {
    public:
    scene_handler(scene_type startup_scene, buz::world_renderer_3d& world_3d, asset_cache& asset_cache_);
    scene_handler() = default;
    scene_handler(const scene_handler& other) = delete;

    ~scene_handler() = default;

    void switch_scene();

    bool has_pending_scene();

    scene& get_current_scene();

    private:

    void set_scene(scene_type scene_t);

    std::unique_ptr<scene> m_current_scene;

    buz::world_renderer_3d& m_world_3d;
    asset_cache& m_asset_cache;
};


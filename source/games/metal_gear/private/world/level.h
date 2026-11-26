#pragma once

#include "renderer_3d/model.h"
#include "collision/collider.h"
#include "collision/collision_world.h"
#include "assets/textured_model.h"

#include <vector>

namespace buz{
    class world_renderer_3d;
}


class level{
    public:
    level(const textured_model& map);
    level(const level& other) = delete;
    ~level() = default;
    
    const collision_world& get_collision_world() const;
    const std::vector<buz::vec3>& get_path(int guard_index) const;
    
    void draw_3d(buz::world_renderer_3d& world_3d) const;
private:
    std::vector<collider::box> create_boxes(const buz::model& map_model);
    std::vector<std::vector<buz::vec3>> create_paths(const buz::model& map_model);

    const textured_model& m_map;

    collision_world m_collision_world;
    std::vector<buz::mat4> m_local_m4s;
	std::vector<std::vector<buz::vec3>> m_guard_paths;

};
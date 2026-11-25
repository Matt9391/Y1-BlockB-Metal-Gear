#pragma once

#include "renderer_3d/model.h"
#include "collision/collider.h"
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

    const std::vector<collider::box>& get_solid_boxes() const;

    void draw_3d(buz::world_renderer_3d& world_3d) const;
private:

    const textured_model& m_map;

    std::vector<collider::box> m_solid_boxes;
    std::vector<buz::mat4> m_local_m4s;

};
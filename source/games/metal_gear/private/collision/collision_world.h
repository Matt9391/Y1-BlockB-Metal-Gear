#pragma once

#include "collision/collider.h"
#include <vector>


class collision_world{
public:
    collision_world(const std::vector<collider::box> boxes);

    const std::vector<collider::box>& get_boxes() const;

    buz::vec3 resolve_move(const collider::box& box, buz::vec3 displacement) const;

private:
    const std::vector<collider::box> m_boxes;

};
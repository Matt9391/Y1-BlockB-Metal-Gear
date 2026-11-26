#pragma once

#include "core/math.h"

#include <vector>

class guard_brain{
public:
    guard_brain(std::vector<buz::vec3> path, float arrival_radius);
    ~guard_brain() = default;

    buz::vec3 compute_direction(const buz::vec3 guard_feet_pos) const;

    void advance_if_arrived(const buz::vec3 guard_feet_pos);

private:
    int m_next_point_index;
    std::vector<buz::vec3> m_path;
    float m_arrival_radius;
};
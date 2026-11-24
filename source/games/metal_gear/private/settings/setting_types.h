#pragma once

#include <array>

struct snake_settings{
    std::array<float, 3> spawn_position;
    float walk_speed_m_per_s;
    float turn_speed_deg_per_s;
};
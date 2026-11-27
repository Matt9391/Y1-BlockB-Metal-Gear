#pragma once

#include <array>

struct snake_settings{
    std::array<float, 3> spawn_position;
    float walk_speed_m_per_s;
    float turn_speed_deg_per_s;
};

struct guard_settings{
    float walk_speed_m_per_s;
    float turn_speed_deg_per_s;
    float arrival_radius;
    float idle_time_s;
};

struct cone_settings{
    float view_distance_m;
    float cone_full_angle_deg;
};
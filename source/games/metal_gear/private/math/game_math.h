#pragma once
#include "core/math.h"

struct view_cone;

namespace game_math
{
    /// @brief return a vector that points from start to end
    /// @param from start
    /// @param to end
    /// @return to - from, with y=0 
    buz::vec3 flat_vector_to(const buz::vec3& from, const buz::vec3& to); 

    bool is_point_in_cone(const buz::vec3& apex, const buz::vec3& forward, const view_cone& cone, const buz::vec3& point);

    float remap(float value,float in_min,float in_max,float out_min,float out_max);

} // namespace game_math

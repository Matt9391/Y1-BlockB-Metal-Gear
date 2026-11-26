#pragma once
#include "core/math.h"


namespace game_math
{
    /// @brief return a vector that points from start to end
    /// @param from start
    /// @param to end
    /// @return to - from, with y=0 
    buz::vec3 flat_vector_to(const buz::vec3& from, const buz::vec3& to); 
} // namespace game_math

#pragma once

#include "core/math.h"

namespace buz{
    class input;
}





namespace player_control{
    buz::vec3 compute_direction(const buz::input& input, const buz::vec3 forward, const buz::vec3 right);
}
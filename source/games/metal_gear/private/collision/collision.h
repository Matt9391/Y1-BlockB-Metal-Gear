#pragma once

#include "core/math.h"


namespace collision{

    buz::vec3 aabb_overlap(buz::vec3 a_pos, buz::vec3 a_size, buz::vec3 b_pos, buz::vec3 b_size);

    bool aabb_is_overlapping(buz::vec3 overlap);

}
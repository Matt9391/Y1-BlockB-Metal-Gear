#pragma once

#include <vector>
#include "core/math.h"
#include "collider.h"


namespace collision{
    extern bool collision_disabled; //to remove

    buz::vec3 aabb_overlap(buz::vec3 a_pos, 
                           buz::vec3 a_size, 
                           buz::vec3 b_pos, 
                           buz::vec3 b_size);

    bool aabb_is_overlapping(buz::vec3 overlap);

    buz::vec3 resolve_boxes(buz::vec3 centre, 
                            const buz::vec3 size, 
                            buz::vec3 displacement,  
                            const std::vector<collider::box>& boxes);

}
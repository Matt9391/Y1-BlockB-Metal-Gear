#include "game_math.h"

#include "view_cone.h"

namespace game_math
{
    buz::vec3 flat_vector_to(const buz::vec3& from, const buz::vec3& to) {
        buz::vec3 from_to = to - from;
        from_to.y = 0;
        return from_to;
    }

    bool is_point_in_cone(const buz::vec3& apex, const buz::vec3& forward, const view_cone& cone, const buz::vec3& point){
        buz::vec3 apex_to_point = flat_vector_to(apex, point);
        
        //distance test
        if(buz::dot(apex_to_point, apex_to_point) > (cone.view_distance_m * cone.view_distance_m))
            return false;

        //angle test
        float length = buz::length(apex_to_point);
        if(length == 0.f)
            return true; //you're on the guard man

        if(buz::dot(forward, apex_to_point) / length < std::cos(buz::radians(cone.cone_full_angle_deg / 2.f)))
            return false;

        return true;
    }
} 
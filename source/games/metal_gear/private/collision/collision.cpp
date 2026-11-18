#include "collision.h"
#include <cmath>

namespace collision{

    buz::vec3 aabb_overlap(buz::vec3 a_centre, buz::vec3 a_size, buz::vec3 b_centre, buz::vec3 b_size){

        buz::vec3 overlap{0.f, 0.f, 0.f};

        buz::vec3 a_half_size = a_size / 2.f;
        buz::vec3 b_half_size = b_size / 2.f;
        
        overlap.x = std::fmin(a_centre.x + a_half_size.x, b_centre.x + b_half_size.x ) - std::fmax(a_centre.x - a_half_size.x, b_centre.x - b_half_size.x ) ;
        overlap.y = std::fmin(a_centre.y + a_half_size.y, b_centre.y + b_half_size.y ) - std::fmax(a_centre.y - a_half_size.y, b_centre.y - b_half_size.y ) ;
        overlap.z = std::fmin(a_centre.z + a_half_size.z, b_centre.z + b_half_size.z ) - std::fmax(a_centre.z - a_half_size.z, b_centre.z - b_half_size.z ) ;

        return overlap;
    }

    bool aabb_is_overlapping(buz::vec3 overlap){
        return (overlap.x > 0.f && overlap.y > 0.f && overlap.z > 0.f);
    }

}
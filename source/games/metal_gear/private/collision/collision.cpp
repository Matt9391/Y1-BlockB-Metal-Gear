#include "collision.h"
#include <cmath>
#include "core/log.h"


namespace collision{   
    bool collision_disabled = false;

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

    buz::vec3 resolve_boxes(buz::vec3 centre, const buz::vec3 size, buz::vec3 displacement, const std::vector<collider::box>& boxes){
        if(collision::collision_disabled) {
            buz::log::info("collision disabled");
            return {centre + displacement};
        }
        
        //TODO: replace 0,2 with axe::z, axe::z
        centre = resolve_single_axe_over_boxes(0, centre, size, displacement,boxes); //new x moved centre
        centre = resolve_single_axe_over_boxes(2, centre, size, displacement,boxes); //new z moved centre

        return centre; //new centre 

    }

    buz::vec3 resolve_single_axe_over_boxes(int axis_index, buz::vec3 centre, buz::vec3 size, buz::vec3 displacement, const std::vector<collider::box>& boxes){
        assert((axis_index > - 1 && axis_index < 3) && "[collision.cpp] wrong axis indexed for collision resolution");

        if(displacement[axis_index] == 0) return centre;

        buz::vec3 half_size = size / 2.f;

        
        int sign = displacement[axis_index] < 0 ? -1 : 1;
        for(const collider::box& b : boxes){
            buz::vec3 centre_moved = centre;
            centre_moved[axis_index] += displacement[axis_index]; 
            buz::vec3 overlap = aabb_overlap(centre_moved, size, b.centre, b.size);
            buz::vec3 b_half_size = b.size / 2.f;
            if(aabb_is_overlapping(overlap)){
                
                displacement[axis_index] = 0;
                centre[axis_index] = b.centre[axis_index] - ((b_half_size[axis_index] + half_size[axis_index]) * sign);               
                
            }
        }
        centre[axis_index] += displacement[axis_index];

        return centre;
    }

}
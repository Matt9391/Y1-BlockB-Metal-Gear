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
            return {centre + displacement - buz::vec3{0.f, size.y / 2.f, 0.f}};
        }
        buz::vec3 half_size = size / 2.f;
        
        int sign_x = displacement.x < 0 ? -1 : 1;
        //TODO: make a function that takes a vector and work only on 1 axe (ITS FOR THE ILO)
        for(const collider::box& b : boxes){
            buz::vec3 centre_x_moved = centre + buz::vec3{displacement.x, 0.f, 0.f}; 
            buz::vec3 overlap = aabb_overlap(centre_x_moved, size, b.centre, b.size);
            buz::vec3 b_half_size = b.size / 2.f;
            if(aabb_is_overlapping(overlap)){
                
                displacement.x = 0;
                centre.x = b.centre.x - ((b_half_size.x + half_size.x) * sign_x);               
                
            }
        }
        
        int sign_z = displacement.z < 0 ? -1 : 1;
        
        centre.x += displacement.x;

        for(const collider::box& b : boxes){
            buz::vec3 centre_z_moved = centre + buz::vec3{0.f, 0.f, displacement.z}; 
            buz::vec3 overlap = aabb_overlap(centre_z_moved, size, b.centre, b.size);
            buz::vec3 b_half_size = b.size / 2.f;
            if(aabb_is_overlapping(overlap)){
                
                displacement.z = 0;
                centre.z = b.centre.z - ((b_half_size.z + half_size.z) * sign_z);               

            }
        }

        centre.z += displacement.z;
                        //return the feet position so it remove the offset of the centre
        return centre - buz::vec3{0.f, size.y / 2.f, 0.f}; //new position 

    }

}
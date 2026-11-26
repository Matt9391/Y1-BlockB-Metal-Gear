#include "guard_brain.h"
#include "math/game_math.h"
#include "core/assert.h"

guard_brain::guard_brain(std::vector<buz::vec3> path, float arrival_radius)
	: m_next_point_index(0)
	, m_path(std::move(path))
    , m_arrival_radius(arrival_radius)
{   
    
    if(m_path.size() < 2){
        buz::fatal_error("[guard_brain] impossible to define a path, not enough points {}", m_path.size());
    }
}

buz::vec3 guard_brain::compute_direction(const buz::vec3 guard_feet_pos) const
{
    const buz::vec3 dir = game_math::flat_vector_to(guard_feet_pos, m_path[m_next_point_index]);

    if(buz::length(dir) != 0){
        return buz::normalize(dir);
    }

    return {0.f,0.f,0.f};
}

void guard_brain::advance_if_arrived(const buz::vec3 guard_feet_pos){
    const buz::vec3 guard_to_point = game_math::flat_vector_to(guard_feet_pos, m_path[m_next_point_index]);
    if(buz::length(guard_to_point) < m_arrival_radius){
        m_next_point_index = (m_next_point_index + 1) % m_path.size();
    }
}





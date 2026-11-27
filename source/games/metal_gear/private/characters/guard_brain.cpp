#include "guard_brain.h"
#include "math/game_math.h"
#include "core/assert.h"
#include "core/log.h"

namespace{
    bool should_move(guard_state state){
		switch (state)
		{
		case guard_state::patrol:
		case guard_state::chase:
			return true;
		case guard_state::idle:
		case guard_state::spotted:
			return false;
			
		default:
			buz::log::error("[guard_brain] invalid state checked for movement");
			break;
		}

		return false;
	}
}


guard_brain::guard_brain(std::vector<buz::vec3> path, float arrival_radius, float idle_time_s)
	: m_state(guard_state::patrol)
	, m_next_point_index(0)
	, m_path(std::move(path))
	, m_arrival_radius(arrival_radius)
	, m_idle_time_s(idle_time_s)
	, m_time_elapsed_s(0.f)
{
	if (m_path.size() < 2)
	{
		buz::fatal_error("[guard_brain] impossible to define a path, not enough points {}", m_path.size());
	}
}

void guard_brain::on_update(float dt, const buz::vec3& guard_feet_pos, bool sees_snake){
	switch (m_state)
	{
	case guard_state::idle:
		look_around(dt);
		break;
	case guard_state::patrol:
		update_patrolling(guard_feet_pos, sees_snake);
		break;
	case guard_state::spotted:
		look_around(dt);
		break;
	case guard_state::chase:	
		/* code */
		break;
	
	default:
		buz::log::error("guard state not recognized");
		break;
	}
}




buz::vec3 guard_brain::compute_direction(const buz::vec3 guard_feet_pos) const
{
	if (!should_move(m_state))
	{
		return { 0.f, 0.f, 0.f };
	}
	
	const buz::vec3 dir = game_math::flat_vector_to(guard_feet_pos, m_path[m_next_point_index]);
	
	if (buz::length(dir) != 0)
	{
		return buz::normalize(dir);
	}
	
	return { 0.f, 0.f, 0.f };
}

guard_state guard_brain::get_state() const{
	return m_state;
}


void guard_brain::advance_if_arrived(const buz::vec3 guard_feet_pos)
{
	const buz::vec3 guard_to_point = game_math::flat_vector_to(guard_feet_pos, m_path[m_next_point_index]);
	if (buz::length(guard_to_point) < m_arrival_radius)
	{		
		m_next_point_index = (m_next_point_index + 1) % m_path.size();
		set_state(guard_state::idle);
	}
}

void guard_brain::update_patrolling(const buz::vec3& guard_feet_pos, bool sees_snake){
	if(sees_snake){
		set_state(guard_state::spotted);
		return;
	}
	advance_if_arrived(guard_feet_pos);
}

void guard_brain::set_state(guard_state next){
	m_state = next;
}

void guard_brain::look_around(float dt){
	m_time_elapsed_s += dt;
	if (m_time_elapsed_s < m_idle_time_s)
	{
		return;
	}
	
	m_time_elapsed_s = 0.f;
	set_state(guard_state::patrol);
}
#pragma once

#include "core/math.h"

#include <vector>


enum class guard_state{
	idle,
	patrol,
	spotted,
    chase
};

namespace{
    bool should_move(guard_state state);
}

class guard_brain{
public:
    guard_brain(std::vector<buz::vec3> path, float arrival_radius, float idle_time_s);
    ~guard_brain() = default;

    void on_update(float dt, const buz::vec3& guard_feet_pos, bool sees_snake);

    buz::vec3 compute_direction(const buz::vec3 guard_feet_pos) const;

    void advance_if_arrived(const buz::vec3 guard_feet_pos);

private:
    void update_patrolling(const buz::vec3& guard_feet_pos, bool sees_snake);
    void set_state(guard_state next);

    void look_around(float dt);
    
    guard_state m_state;

    int m_next_point_index;
    std::vector<buz::vec3> m_path;
    float m_arrival_radius;
    const float m_idle_time_s;
    float m_time_elapsed_s;

};  
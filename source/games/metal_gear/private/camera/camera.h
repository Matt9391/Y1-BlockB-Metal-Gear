#pragma once

#include "core/math.h"

class camera
{
public:
	camera(float fov, buz::vec3 base_offset);

    buz::mat4 get_view_matrix() const;
    buz::mat4 get_projx_matrix(buz::vec2 window_size) const;
    
    void set_target(const buz::vec3& target);

private:
	void set_offset(const buz::vec3& offset);

    float m_fov;
    buz::vec3 m_offset;
    buz::vec3 m_target;

    float m_near_limit;
    float m_far_limit;
};
